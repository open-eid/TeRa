/*
 * main_window.cpp
 *
 *  Created on: Nov 15, 2016
 */

#include "main_window.h"

#include <iostream>

#include <QCloseEvent>
#include <QDesktopServices>
#include <QDirIterator>
#include <QFile>
#include <QGraphicsProxyWidget>
#include <QMessageBox>
#include <QRegion>
#include <QProcess>
#include <QThreadPool>








#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <QSslKey>
#include "common/QPCSC.h"
#include "common/SslCertificate.h"
//#include <AboutDialog.h>



#include "disk_crawler.h"
#include "settings_window.h"
#include "../src/libdigidoc/Configuration.h"

namespace {
    static int PP_TS_TEST = 100;
    static int PP_SEARCH = 500;
    static int PP_TS = 400;

    void fixFontSize(QWidget* w) {
        QFont font = w->font();
        font.setPointSizeF(font.pointSizeF() * 96 / w->logicalDpiX());
        w->setFont(font);
    }

    void fixFontSizeInStyleSheet(QWidget* w) {
        QRegExp reg(".*font-size: *([0-9]+(.[0-9]+)?)pt;.*");
        QString stylesheet = w->styleSheet();
        reg.exactMatch(stylesheet);
        int pos = reg.pos(1);
        if (pos >= 0) {
            QString match = reg.cap(1);
            qreal fontsize = match.toFloat();
            qreal fontsizeNew = fontsize * 96 / w->logicalDpiX();
            QString replacement = QString::number(fontsizeNew, 'f', 2);
            stylesheet.replace(pos, match.size(), replacement);
            w->setStyleSheet(stylesheet);
        }
    }
}

namespace ria_tera {

TeraMainWin::TeraMainWin(QWidget *parent) :
    QWidget(parent),
    initDone(false),
    showingIntro(false),
    nameGen("ddoc", "asics"), // TODO consts
    stamper(*this, nameGen, false),
    settingsWin(NULL),
    appTranslator(this),
    btnIntroAccept(NULL),
    btnIntroReject(NULL)
{
    setupUi(this);
    fixFontSizeInStyleSheet(btnStamp);
    fixFontSizeInStyleSheet(cancelProcess);
    fixFontSizeInStyleSheet(btnReady);
    fixFontSize(logText);

    settings->setStyleSheet("QPushButton:disabled"
            "{ color: gray }");

    introButtonBox->setStandardButtons(QDialogButtonBox::NoButton);
    btnIntroAccept = introButtonBox->addButton(QString(), QDialogButtonBox::AcceptRole);
    btnIntroReject = introButtonBox->addButton(QString(), QDialogButtonBox::RejectRole);

    if (processor.isShowIntroPage()) setPage(PAGE::INTRO);
    else setPage(PAGE::START);

    connect(introButtonBox, SIGNAL(accepted()), this, SLOT(introAccept()));
    connect(introButtonBox, SIGNAL(rejected()), this, SLOT(introReject()));

    connect(btnStamp, SIGNAL (clicked()), this, SLOT (handleStartStamping()));
    connect(settings, SIGNAL(clicked()), this, SLOT(handleSettings()));
    connect(about, SIGNAL(clicked()), this, SLOT(handleAbout()));
    connect(help, SIGNAL(clicked()), this, SLOT(handleHelp()));
    connect(cancelProcess, SIGNAL(clicked()), this, SLOT(handleCancelProcess()));
    connect(btnReady, SIGNAL(clicked()), this, SLOT(handleReadyButton()));

    connect(&stamper, SIGNAL(timestampingFinished(bool,QString)),
            this, SLOT(timestampingFinished(bool,QString)));
    connect(&stamper.getTimestamper(), SIGNAL(timestampingTestFinished(bool,QByteArray,QString)),
            this, SLOT(timestampingTestFinished(bool,QByteArray,QString)));

    connect(logText, SIGNAL(anchorClicked(const QUrl&)),
            this, SLOT(showLog(QUrl const&)));

    settingsWin.reset(new TeraSettingsWin(this));
    connect(settingsWin.data(), SIGNAL(accepted()),
            this, SLOT(handleSettingsAccepted()));

    filesWin.reset(new FileListWindow(this));
    connect(filesWin.data(), SIGNAL(accepted()),
            this, SLOT(handleFilesAccepted()));
    connect(filesWin.data(), SIGNAL(rejected()),
            this, SLOT(handleFilesRejected()));

    // Translations
    langs << "et" << "en" << "ru";
    QActionGroup *langGroup = new QActionGroup( this );
    for( int i = 0; i < langs.size(); ++i )
    {
        QAction *a = langGroup->addAction( new QAction( langGroup ) );
        a->setData( langs[i] );
        a->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_0 + i );
    }
    addActions( langGroup->actions() );
    connect( languages, SIGNAL(activated(int)), SLOT(slotLanguageChanged(int)) ); // TODO do with actions?
    connect( langGroup, SIGNAL(triggered(QAction*)), SLOT(slotLanguageChanged(QAction*)) );

    loadTranslation("et");

    timestapmping = false;

    ///
    connect(&Configuration::instance(), SIGNAL(finished(bool, const QString&)), this, SLOT(globalConfFinished(bool, const QString&)));
    Configuration::instance().update();
}

TeraMainWin::~TeraMainWin()
{
}

CrawlDiskJob::CrawlDiskJob(TeraMainWin& mainWindow, int jobid, GuiTimestamperProcessor const & processor) :
gui(mainWindow), jobId(jobid), dc(*this, Config::EXTENSION_IN)
{
    dc.addExcludeDirs(processor.exclDirs.toList());

    QList<QString> inDirList = processor.inclDirs.toList(); // TODO checked previously
    for (int i = 0; i < inDirList.size(); ++i) {
        QString dir = inDirList.at(i);
        dc.addInputDir(dir, true);
    }
}

void CrawlDiskJob::run() {
    dc.crawl();
    emit signalFindingFilesDone(jobId);
}

bool CrawlDiskJob::isCanceled() {
    return gui.isCancelled(jobId);
}

bool CrawlDiskJob::processingPath(QString const& path, double progress_percent) {
    if (isCanceled()) return false;
    emit signalProcessingPath(jobId, path, progress_percent);
    return true;
}

bool CrawlDiskJob::excludingPath(QString const& path) {
    if (isCanceled()) return false;
    emit signalExcludingPath(jobId, path);
    return true;
}

bool CrawlDiskJob::foundFile(QString const& path) {
    if (isCanceled()) return false;
    emit signalFoundFile(jobId, path);
    return true;
}


void TeraMainWin::configureRequest(QNetworkRequest& request) {
    qDebug() << "TeraMainWin::configureRequest start";
    QSslCertificate cert = cardSelectDialog->smartCardData.authCert();

    QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
    QList<QSslCertificate> trusted;
    //        for (const QJsonValue &cert : Configuration::instance().object().value("CERT-BUNDLE").toArray())
    //            trusted << QSslCertificate(QByteArray::fromBase64(cert.toString().toLatin1()), QSsl::Der);
    ssl.setCaCertificates(QList<QSslCertificate>());
    ssl.setProtocol(QSsl::TlsV1_0);
    Qt::HANDLE key = cardSelectDialog->smartCard->key();
    if (key)
    {
        qDebug() << "key added <<<<<< ";
        ssl.setPrivateKey(QSslKey(key));
        ssl.setLocalCertificate(cert);
    }
    request.setSslConfiguration(ssl);
    qDebug() << "TeraMainWin::configureRequest end";
}

void TeraMainWin::handleStartStamping() {
    doPin1Authentication();
}

void TeraMainWin::doPin1Authentication() {
    QPCSC& instance = QPCSC::instance();
    QStringList drivers = instance.drivers();
    QStringList readers = instance.readers();


    cardSelectDialog.reset(new IDCardSelectDialog(this));
    connect(cardSelectDialog.data(), SIGNAL(accepted()), this, SLOT(pin1AuthenticaionDone()));
    cardSelectDialog->setVisible(true);
}

void TeraMainWin::pin1AuthenticaionDone() {
qDebug() << "### TeraMainWin::pin1AuthenticaionDone";
    QString reader = cardSelectDialog->smartCardData.reader();
    doTestStamp();
}

void TeraMainWin::doTestStamp() {
    qDebug() << "### TeraMainWin::doTestStamp";
    processor.timeServerUrl = processor.timeServerUrl.trimmed(); // TODO
    if (processor.timeServerUrl.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Time server URL is empty.")); // Error only onve
        return;
    }

    if (!processor.checkInDirListWithMessagebox(this)) return;

    //
    cancel = false;
    processor.foundFiles.clear();
    processor.inFiles.clear();
    timestapmping = true;

    // create log file
    QString error;
    processor.result.reset(new GuiTimestamperProcessor::Result());
    if (!processor.openLogFile(error)) { // TODO  finish message box
        QMessageBox mb(this);
        mb.exec();
    }

    setPage(PAGE::PROCESS);
    settings->setEnabled(false);
    progressBar->setMaximum(PP_TS_TEST + PP_SEARCH + PP_TS);
    progressBar->setValue(0);
    fillProgressBar();

    QByteArray pseudosha256(256/8, '\0');
    QByteArray req = stamper.getTimestamper().getTimestampRequest4Sha256(pseudosha256);
    stamper.getTimestamper().setTimeserverUrl(processor.timeServerUrl, this); // TODO true = ask PIN1 XXXXXXXXXXXXXXXXXXXXXXXXXXXXX
    stamper.getTimestamper().sendTSRequest(req, true); // TODO api is rubbish
}

void TeraMainWin::timestampingTestFinished(bool success, QByteArray resp, QString errString) {
    if (!success) {
        timestampingFinished(false, tr("Test request to Time Server failed. ") + "\n" + errString);
        return;
    }

    if (isCancelled()) {
        doUserCancel();
        return;
    }

    progressBar->setValue(PP_TS_TEST);
    processor.result->progressStage = GuiTimestamperProcessor::Result::SEARCHING_FILES;
    processor.foundFiles.clear();
    processor.inFiles.clear();
    fillProgressBar();

    int newJobId = jobId.fetchAndAddOrdered(1);
    CrawlDiskJob* crawlJob = new CrawlDiskJob(*this, newJobId, processor);
    connect(crawlJob, SIGNAL(signalProcessingPath(int, QString, double)),
        this, SLOT(processProcessingPath(int, QString, double)));
    connect(crawlJob, SIGNAL(signalExcludingPath(int, QString)),
        this, SLOT(processExcludingPath(int, QString)));
    connect(crawlJob, SIGNAL(signalFoundFile(int, QString)),
        this, SLOT(processFoundFile(int, QString)));
    connect(crawlJob, SIGNAL(signalFindingFilesDone(int)),
        this, SLOT(processFindingFilesDone(int)));

    QThreadPool::globalInstance()->start(crawlJob);
}

void TeraMainWin::processProcessingPath(int jobid, QString path, double progress_percent) {
    if (isCancelled(jobid)) return;

    progressBar->setValue((int)(PP_TS_TEST + progress_percent * PP_SEARCH));
    progressBar->setFormat(QObject::tr("Searching") + " " + path + "...");
}

void TeraMainWin::processExcludingPath(int jobid, QString path) {
}

void TeraMainWin::processFoundFile(int jobid, QString path) {
    if (isCancelled(jobid)) return;
    if (!processor.foundFiles.contains(path)) {
        GuiTimestamperProcessor::InFileData ifd(path);
        processor.foundFiles.insert(path, ifd);
        processor.inFiles.append(path);
    }
    fillProgressBar();

    if (processor.logfile) {
        processor.logfile->getStream() << "Found " << path << endl;
    }
}

void TeraMainWin::processFindingFilesDone(int jobid) {
    if (isCancelled(jobid)) return;
    doFindingFilesDone();
}

void TeraMainWin::doFindingFilesDone() {
    if (isCancelled()) {
        doUserCancel();
        return;
    }

    progressBar->setValue(PP_TS_TEST + PP_SEARCH);
    progressBar->setFormat("");
    processor.result->progressStage = GuiTimestamperProcessor::Result::CONVERTING_FILES;
    processor.result->cntFound = processor.inFiles.size();
    fillProgressBar();

    if (processor.inFiles.size() > 0 && processor.previewFiles) {
        processor.initializeFilePreviewWindow(*filesWin);
        filesWin->open();
    } else {
        startStampingFiles();
    }
}

void TeraMainWin::startStampingFiles() {
    // TODO comment what callbacks follow in this process
    // TODO separate thread for size estimates
    {
        QMap<QString,qint64> filesizesPerPartitition;
        for (int i = 0; i < processor.inFiles.size(); ++i) {
            QString filePath = processor.inFiles.at(i);
            auto it = processor.foundFiles.find(filePath);
            if (processor.foundFiles.end() != it) {
                auto& data(it.value());
                auto& filesizeOnPart(filesizesPerPartitition[data.partitionPath]);
                filesizeOnPart += data.filesize;
            }
        }

        bool spaceIssue = false;
        QString sizeInfo;
#ifdef TERA_USE_UNIX_STORAGE_INFO
#else
        for (auto it = filesizesPerPartitition.begin(); it != filesizesPerPartitition.end(); ++it) {
            QString partition = it.key();
            qint64 filesTotalSize = it.value();
            qint64 partitionAvailableSize = QStorageInfo(partition).bytesAvailable();
            if (filesTotalSize > partitionAvailableSize) {
                spaceIssue = true;
                sizeInfo = QString(tr(" * '%1': vaba ruumi %2, ruumi vaja %3 (hinnanguline)\n")).
                    arg(hrPath(partition), hrSize(partitionAvailableSize), hrSize(filesTotalSize));
            }
        }
#endif

        if (spaceIssue) {
            QString errorMsg = QString() +
                tr("Leitud DDOC failide kogumaht ületab kettal oleva vaba ruumi:\n\n") +
                sizeInfo +
                tr("\nÜletembeldatud failid ei pruugi kettale ära mahtuda.");
            QString message = errorMsg + tr("\n\nKatkestada tembeldamine?");

            QMessageBox::StandardButtons button = QMessageBox::warning(this, this->windowTitle(), message, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (QMessageBox::Yes == button) {
                doUserCancel(errorMsg);
            }
            return;
        }
    }

    nameGen.setOutExt(processor.outExt); // TODO threading issues?
    stamper.startTimestamping(processor.timeServerUrl, processor.inFiles);
}

bool TeraMainWin::processingFile(QString const& pathIn, QString const& pathOut, int nr, int totalCnt) {
    if (isCancelled()) {
        doUserCancel();
        return false;
    }
    return true;
}

bool TeraMainWin::processingFileDone(QString const& pathIn, QString const& pathOut, int nr, int totalCnt, bool success, QString const& errString) {
    if (isCancelled()) {
        doUserCancel();
        return false;
    }

    if (processor.logfile) {
        QString log1 = QString("[%1/%2]").arg(QString::number(nr+1), QString::number(totalCnt));
        QString log2 = QString("%1 -> %2").arg(pathIn, pathOut);
        if (success) {
            processor.logfile->getStream() << log1 << " DONE " << log2 << endl;
        } else {
            processor.logfile->getStream() << log1 << " FAILED " << log2 << " : " << errString << endl;
        }
    }

    processor.result->progressStage = GuiTimestamperProcessor::Result::DONE;
    processor.result->progressConverted = nr+1;
    if (success) processor.result->progressSuccess++;
    else processor.result->progressFailed++;

    progressBar->setValue(PP_TS_TEST + PP_SEARCH + (int)(1.0*PP_TS*(nr+1) / totalCnt));
    progressBar->setFormat("");
    fillProgressBar();
    return true;
}

void TeraMainWin::globalConfFinished(bool changed, const QString &error) {
    initDone = true;
    progressBarDnldConf->setValue(100);
    if (!showingIntro) setPage(PAGE::START);
    processor.processGlobalConfiguration();
}

void TeraMainWin::timestampingFinished(bool success, QString errString) {
    if (processor.result) {
        if (success) {
            processor.result->success = true;
            processor.result->cnt = processor.inFiles.size(); // TODO
        }
        else {
            processor.result->success = false;
            processor.result->error = errString;
            logText->clear();
        }
    }
    fillProgressBar();
    fillDoneLog();

    if (processor.logfile) processor.logfile->close();

    processor.inFiles.clear();
    timestapmping = false;
    if (cancel.load()) setPage(PAGE::START);
    else setPage(PAGE::READY);
    settings->setEnabled(true);
}

void TeraMainWin::closeEvent(QCloseEvent *event) {
    QMessageBox::StandardButton res = QMessageBox::Yes;

    if (timestapmping) {
        res = QMessageBox::question(this, windowTitle(),
            tr("Timestamping is not finished. Are you sure?\n"),
            QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
            QMessageBox::Yes);
    }

    if (res == QMessageBox::Yes) {
        doUserCancel();
        event->accept();
    }
    else {
        event->ignore();
    }
}

void TeraMainWin::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
    if (QEvent::LanguageChange == event->type()) {
        fillProgressBar();
        fillDoneLog();
    }
}

void TeraMainWin::fillProgressBar() {
    progressText->clear();
    if (!processor.result) return;

    if (GuiTimestamperProcessor::Result::TESTING_TIME_SERVER == processor.result->progressStage) {
        progressText->setText(tr("Testing Time Server..."));
    } else if (GuiTimestamperProcessor::Result::SEARCHING_FILES == processor.result->progressStage) {
        progressText->setText(tr("Searching DDOC files. %1 found so far...").arg(processor.inFiles.size()));
    } if (GuiTimestamperProcessor::Result::CONVERTING_FILES == processor.result->progressStage) {
        progressText->setText(tr("Found %1 DDOC files. %2 left to be converted...").
            arg(QString::number(processor.inFiles.size()),
                QString::number(processor.inFiles.size() - processor.result->progressConverted)) );
    }
}

void TeraMainWin::fillDoneLog() {
    logText->clear();
    if (!processor.result) return;

    QTextCharFormat format = logText->currentCharFormat();
    format.setAnchor(false);
    format.setAnchorHref(QString());
    format.setFontUnderline(false);
    logText->setCurrentCharFormat(format);

    if (!processor.result->success) {
        logText->append(tr("Viga:"));
        logText->append(processor.result->error);
        return;
    }

    logText->append(tr("DDOC failide konverteerimine lõppes"));
    logText->append(tr("DDOC faile leitud: %1").arg(QString::number(processor.result->cntFound)));
    if (processor.result->cntFound != processor.result->cnt)
        logText->append(tr("   millest tembeldamiseks valisite: %1").arg(QString::number(processor.result->cnt)));
    logText->append(tr("DDOC faile konverteeritud: %1").arg(QString::number(processor.result->progressSuccess)));
    if (processor.result->progressFailed > 0) {
        logText->append(tr("Ebaõnnestunud konverteerimisi: %1").arg(QString::number(processor.result->progressFailed)));
    }

    if (processor.logfile) {
        logText->append(tr("Täpsema aruande vaatamiseks vajuta "));

        QString filepath = processor.logfile->filePath();
        QTextCursor cursor = logText->textCursor();
        format.setAnchor(true);
        format.setAnchorHref(filepath);
        format.setFontUnderline(true);

        cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        cursor.insertText(tr("SIIA"), format);
    }
}

void TeraMainWin::handleAbout() {
    //AboutDialog *a = new AboutDialog(this);
    //a->addAction(d->closeAction);
    //a->open();
}

void TeraMainWin::handleHelp() {
    QString url = tr("HTTP_HELP");
    if (!QDesktopServices::openUrl(url)) {
        QMessageBox::warning(this, this->windowTitle(), tr("Couldn't open help URL: ") + url);
    }
}

void TeraMainWin::showLog(QUrl const& link) {
    bool success = QDesktopServices::openUrl(link);

#if defined(Q_OS_MAC)
    if (!success) {
        qDebug() << "Opening log-file " << link.url();
        success = QProcess::startDetached("open", QStringList()  << link.url());
    }
#endif

    if (!success) {
        QMessageBox::warning(this, this->windowTitle(), tr("Couldn't open timestamping log: ") + link.toDisplayString());
    }
}

void TeraMainWin::loadTranslation(QString const& language_short) {
    int idx = langs.indexOf(language_short);
    if (idx < 0) return; // TODO

    if (idx != languages->currentIndex()) languages->setCurrentIndex(idx);
    if (lang == language_short) return;

    lang = language_short;
    if( lang == "en" ) QLocale::setDefault( QLocale( QLocale::English, QLocale::UnitedKingdom ) );
    else if( lang == "ru" ) QLocale::setDefault( QLocale( QLocale::Russian, QLocale::RussianFederation ) );
    else QLocale::setDefault( QLocale( QLocale::Estonian, QLocale::Estonia ) );

    qApp->removeTranslator(&appTranslator);
    appTranslator.load(":/translations/" + lang + ".qm");
    qApp->installTranslator(&appTranslator);

    retranslateUi(this);
    // TODO retranslate other GUIs as well?

    versionLabel->setText(versionLabel->text().arg(qApp->applicationVersion()));
    btnIntroAccept->setText(tr("I agree"));
    btnIntroReject->setText(tr("Cancel"));
}

void TeraMainWin::setPage(PAGE p) {
    if (PAGE::INTRO == p) {
        setBackgroundImg(":/images/background.clean.png");
        introStackedWidget->setCurrentIndex(1);
        showingIntro = true;
        return;
    }

    showingIntro = false;
    setBackgroundImg(":/images/background.png");
    introStackedWidget->setCurrentIndex(0);
    if (PAGE::START == p) {
        if (initDone) {
            stackedMainWidget->setCurrentIndex(0);
            stackedBtnWidget->setCurrentIndex(0);
            logText->setFocus();
        } else {
            stackedMainWidget->setCurrentIndex(2);
        }
    } else if (PAGE::PROCESS == p) {
        stackedMainWidget->setCurrentIndex(1);
    } else if (PAGE::READY == p) {
        stackedMainWidget->setCurrentIndex(0);
        stackedBtnWidget->setCurrentIndex(1);
    }
}

void TeraMainWin::setBackgroundImg(QString path) {
    if (path == backgroundImg) return;
    
    backgroundImg = path;
    QPixmap bkgnd(backgroundImg);
    QPalette palette;
    palette.setBrush(QPalette::Background, bkgnd);
    this->setPalette(palette);
}

void TeraMainWin::handleCancelProcess() {
    doUserCancel();
}

void TeraMainWin::doUserCancel(QString msg) {
    cancel = true;
    QString message = msg;
    if (message.isNull()) {
        message = tr("Operation cancelled by user...");
    }
    timestampingFinished(false, message);
}

void TeraMainWin::handleReadyButton() {
    setPage(PAGE::START);
    logText->clear();
}

void TeraMainWin::introAccept() {
    processor.saveShowIntro(!introSkipCheckBox->isChecked());
    setPage(PAGE::START);
}

void TeraMainWin::introReject() {
    close();
}

void TeraMainWin::handleSettings() {
    processor.initializeSettingsWindow(*settingsWin);
    settingsWin->open();
}

void TeraMainWin::handleSettingsAccepted() {
    processor.readSettings(*settingsWin);
    processor.saveSettings();
}

void TeraMainWin::processEvents() {
    QCoreApplication::processEvents();
}

void TeraMainWin::handleFilesAccepted() {
    processor.copySelectedFiles(*filesWin);
    startStampingFiles();
}

void TeraMainWin::handleFilesRejected() {
    doUserCancel();
}

void TeraMainWin::slotLanguageChanged(int i) {
    loadTranslation( langs[i] ); // TODO check range
}

void TeraMainWin::slotLanguageChanged(QAction* a) {
    loadTranslation( a->data().toString() );
}

bool TeraMainWin::isCancelled() { // TODO
    return cancel.load() != 0;
}

bool TeraMainWin::isCancelled(int jobid) {
    return cancel.load() != 0 || jobid != jobId.load();
}

}
