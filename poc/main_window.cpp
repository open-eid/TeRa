/*
 * TeRa
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
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
#ifdef Q_OS_OSX
    #include <QFileDialog>
#endif


#include "common/AboutDialog.h"
#include "disk_crawler.h"
#include "settings_window.h"
#include "src/libdigidoc/Configuration.h"

#ifdef Q_OS_OSX
    #include "utils_mac.h"
#endif

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
    nameGen(ria_tera::Config::IN_EXTENSIONS, ria_tera::Config::DEFAULT_OUT_EXTENSION),
    stamper(*this, nameGen, false),
    settingsWin(NULL),
    appTranslator(this),
    qtTranslator(this),
    btnIntroAccept(NULL),
    btnIntroReject(NULL)
{
    setupUi(this);
    qApp->installTranslator(&appTranslator);
    qApp->installTranslator(&qtTranslator);
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
    connect(settings, &QPushButton::clicked, this, &TeraMainWin::handleSettings);
    connect(about, SIGNAL(clicked()), this, SLOT(handleAbout()));
    connect(help, SIGNAL(clicked()), this, SLOT(handleHelp()));
    connect(cancelProcess, SIGNAL(clicked()), this, SLOT(handleCancelProcess()));
    connect(btnReady, SIGNAL(clicked()), this, SLOT(handleReadyButton()));

    connect(&stamper, &ria_tera::BatchStamper::timestampingFinished,
            this, &TeraMainWin::timestampingFinished);
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

    QString sysLang = processor.settings.value("Main/Language").toString();
    QString lang = "et";
    if (langs.contains(sysLang)) lang = sysLang;
    loadTranslation(lang);

    timestapmping = false;

    ///
    connect(&Configuration::instance(), SIGNAL(finished(bool, const QString&)), this, SLOT(globalConfFinished(bool, const QString&)), Qt::QueuedConnection);
    connect(&Configuration::instance(), SIGNAL(networkError(const QString&)), this, SLOT(globalConfNetworkError(const QString&)));
    Configuration::instance().update();
}

TeraMainWin::~TeraMainWin() = default;

CrawlDiskJob::CrawlDiskJob(TeraMainWin& mainWindow, int jobid, GuiTimestamperProcessor const & processor, QStringList const &extensions) :
gui(mainWindow), jobId(jobid), dc(*this, extensions)
{
    dc.addExcludeDirs(processor.exclDirs.toList());
    for (const QString &dir: processor.getInclDirList()) // TODO checked previously
        dc.addInputDir(dir, true);
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


void TeraMainWin::handleStartStamping() {
    QString url = processor.timeServerUrl.trimmed();
    bool useIDCardAuthentication = idCardAuth.useIDAuth(url);

    stamper.getTimestamper().setTimeserverUrl(url, (useIDCardAuthentication ? &idCardAuth : nullptr));

    if (!checkSettingsWithGUI()) {
        return;
    }

#ifdef Q_OS_OSX
    QScopedPointer<QSet<QString>> nullVal;
    processor.resetGrants(nullVal, nullVal);

    MacUtils mu;
    QSet<QString> deniedDirs;
    for (const QString &dirPath: processor.getInclDirList()) {
        if (!ria_tera::isSubfolder(dirPath, deniedDirs)) {
            if (!mu.askPermissions(dirPath.toUtf8().constData())) {
                deniedDirs.insert(dirPath);
            }
        }
    }
    if (!deniedDirs.isEmpty() && !grantPermissions(deniedDirs)) {
        QMessageBox::critical(this, tr("Error"),
                              tr("No directories selected for search!") );
        return;
    }
#endif

    if (useIDCardAuthentication) {
        doPin1Authentication();
    } else {
        doTestStamp();
    }
}

#ifdef Q_OS_OSX
bool TeraMainWin::grantPermissions(QSet<QString> const& deniedDirs) {
    QScopedPointer<QSet<QString>> revoked(new QSet<QString>());
    QScopedPointer<QSet<QString>> granted(new QSet<QString>());

    if (!QSettings().value("hideAccess").toBool()) {
        QMessageBox msgBox(QMessageBox::Information, tr("Access rights"),
            tr("SANDBOX_MESSAGE"),
            0, this);
        msgBox.addButton(tr("OK"), QMessageBox::AcceptRole);
        QCheckBox *cb = new QCheckBox(tr("Do not show this message again"));
        cb->setCheckState(Qt::CheckState::Checked);
        msgBox.setCheckBox(cb);
        msgBox.exec();
        if (cb->checkState() == Qt::CheckState::Checked) {
            QSettings().setValue("hideAccess", true);
        }
    }

    // Ask folder grants (new folders can be selected by user)
    for (auto it = deniedDirs.begin(); it != deniedDirs.end(); ++it) {
        // macOS file dialog has no title
        QString newPath = QFileDialog::getExistingDirectory(this, "", *it);
        if (newPath != *it) {
            revoked->insert(*it);
            if (!newPath.isNull()) {
                granted->insert(newPath);
            }
        }
    }

    processor.resetGrants(revoked, granted);
    return !processor.getInclDirList().isEmpty();
}
#endif

bool TeraMainWin::checkSettingsWithGUI() {
    processor.timeServerUrl = processor.timeServerUrl.trimmed(); // TODO
    if (processor.timeServerUrl.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Time server URL is empty.")); // Error only onve
        return false;
    }

    if (!processor.checkInDirListWithMessagebox(this)) {
        bool openDirSelector = false; // TODO
#ifdef Q_OS_MAC
        openDirSelector = true;
#endif
        handleSettingsFromPage(TeraSettingsWin::PAGE::INPUT_DIR, openDirSelector);
        return false;
    }

    return true;
}

void TeraMainWin::doPin1Authentication() {
    if (cardSelectDialog.isNull()) {
        cardSelectDialog.reset(new IDCardSelectDialog(this));
        connect(cardSelectDialog.data(), SIGNAL(accepted()), this, SLOT(pin1AuthenticaionDone()));
    }
    cardSelectDialog->open();
}

void TeraMainWin::pin1AuthenticaionDone() {
    idCardAuth.setAuthCert(cardSelectDialog->smartCardData.cert(), cardSelectDialog->smartCard->key()); // TODO API
    doTestStamp();
}

void TeraMainWin::doTestStamp() {
    if (!checkSettingsWithGUI()) return;

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

    stamper.getTimestamper().sendTSRequest(req, true); // TODO api is rubbish
}

void TeraMainWin::timestampingTestFinished(bool success, QByteArray resp, QString errString) {
    if (!success) {
        timestampingFinished(BatchStamper::FinishingDetails::error(tr("Test request to Time Server failed. ") + "\n" + errString));
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

    int newJobId = jobId.fetchAndAddOrdered(1)+1;
    selectedExtensions.clear();
    if (processor.stampDDoc) selectedExtensions.append(Config::EXTENSION_DDOC);
    if (processor.stampBDoc) selectedExtensions.append(Config::EXTENSION_BDOC);
    if (processor.logfile) {
        processor.logfile->getStream() << "Searching for extensions *.(" << selectedExtensions.join(", ") << ")." << endl;
    }

    CrawlDiskJob* crawlJob = new CrawlDiskJob(*this, newJobId, processor, selectedExtensions);
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
#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
#else
        for (auto it = filesizesPerPartitition.begin(); it != filesizesPerPartitition.end(); ++it) {
            QString partition = it.key();
            qint64 filesTotalSize = it.value();
            qint64 partitionAvailableSize = QStorageInfo(partition).bytesAvailable();
            if (-1 == partitionAvailableSize) {
                // Ignoring UNC path issue or any
            } else if (filesTotalSize > partitionAvailableSize) {
                spaceIssue = true;
                sizeInfo = QString(tr("* %1: free space %2, space needed %3 (approximately)")).
                    arg(hrPath(partition), hrSize(partitionAvailableSize), hrSize(filesTotalSize)) + "\n";
            }
        }
#endif

        if (spaceIssue) {
            QString errorMsg = QString() +
                tr("The space needed to timestamp all the DDOC files found exceeds the amount of free space found:\n\n") +
                sizeInfo +
                tr("\nTimestamped files might not fit on disk.");
            QString message = errorMsg + "\n\n" + tr("Abort timestamping?");

            QMessageBox::StandardButtons button = QMessageBox::warning(this, this->windowTitle(), message, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (QMessageBox::Yes == button) {
                doUserCancel(errorMsg);
                return;
            }
            // Continue timestamping with a space issue
        }
    }

    nameGen.setOutExt(processor.outExt); // TODO threading issues?
    stamper.startTimestamping(processor.timeServerUrl, processor.inFiles); // TODO url not neccessary
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

    processor.result->progressConverted = nr+1;
    if (success) processor.result->progressSuccess++;
    else processor.result->progressFailed++;
    processor.result->progressUnprocessed = totalCnt - processor.result->progressConverted;

    progressBar->setValue(PP_TS_TEST + PP_SEARCH + (int)(1.0*PP_TS*(nr+1) / totalCnt));
    progressBar->setFormat("");
    fillProgressBar();
    return true;
}

QVector<int> getVersionAsArray(QString const& ver) {
    QVector<int> res;
    QStringList verList = ver.split('.');
    for (int i = 0; i < verList.size(); ++i) {
        int num = verList[i].toInt();
        res.append(num);
    }
    return res;
}

static bool isSupported(QString const& ver, QString const& minver) {
    QVector<int> ver_v = getVersionAsArray(ver);
    QVector<int> minver_v = getVersionAsArray(minver);

    int l = qMin(ver_v.size(), minver_v.size());
    for (int i = 0; i < l; ++i) {
        if (ver_v[i] > minver_v[i]) return true;
        if (ver_v[i] < minver_v[i]) {
            return false;
        }
    }

    return true;
}

void TeraMainWin::globalConfFinished(bool changed, const QString &error) {
    initDone = true;
    progressBarDnldConf->setValue(100);
    if (!showingIntro) setPage(PAGE::START);
    processor.processGlobalConfiguration();

    idCardAuth.addTrustedCerts(processor.config.getTrustedHttpsCerts());

    QString minSupported = processor.minSupportedVersion;
    QString appVersion = QCoreApplication::applicationVersion();
    if (!isSupported(appVersion, minSupported)) {
        QString msg = tr("Your version of the software (%1) is not supported any more.\n\nMinimum supported version is %2.\n\nPlease upgrade your software from https://installer.id.ee/").
            arg(appVersion, minSupported);
        QMessageBox::critical(this, tr("Version check"), msg);
        QCoreApplication::exit(1);
    }
}

void TeraMainWin::globalConfNetworkError(const QString &error) {
    QString msg;
    msg = tr("NO_NETWORK_MSG").arg(error);
    QMessageBox::warning(this, tr("Error downloading configuration"), msg);
}

void TeraMainWin::timestampingFinished(BatchStamper::FinishingDetails details) {
    if (processor.result) {
        processor.result->progressStage = GuiTimestamperProcessor::Result::DONE;
        if (details.success) {
            processor.result->success = true;
            processor.result->cnt = processor.inFiles.size(); // TODO
        }
        else {
            processor.result->success = false;
            processor.result->isSystemError = !details.userCancelled;
            if (details.userCancelled && details.errString.isNull()) {
                processor.result->error = tr("Operation cancelled by user...");
            } else {
                processor.result->error = details.errString;
            }
            logText->clear();
        }
    }
    fillProgressBar();
    fillDoneLog();

    if (processor.logfile) {
        if (details.success) {
            if (0 == processor.inFiles.size()) processor.logfile->getStream() << "No *.(" << selectedExtensions.join(", ") << ") files selected for timestamping." << endl;
        } else {
            if (details.userCancelled) {
                processor.logfile->getStream() << "Operation cancelled by user" << endl;
            } else {
                processor.logfile->getStream() << "Operation cancelled with error: " << details.errString << endl;
            }
        }

        processor.logfile->close();
    }

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
        QString prefix = "";
        if (processor.result->isSystemError) prefix = tr("Error:");
        logText->append("<font color='red'>" + prefix + " " + processor.result->error + "</font>");
        logText->append("");
    } else {
        logText->append(tr("Finished timestamping DDOC files"));
    }

    if (processor.result->cntFound >= 0) {
        logText->append(tr("DDOC files found: %1").arg(QString::number(processor.result->cntFound)));
        if (processor.result->cntFound != processor.result->cnt && processor.result->cnt >= 0)
            logText->append(tr("   of which %1 where chosen for timestamping").arg(QString::number(processor.result->cnt)));
        logText->append(tr("DDOC files timestamped: %1").arg(QString::number(processor.result->progressSuccess)));
        if (processor.result->progressFailed > 0) {
            logText->append(tr("Failed timestampings: %1").arg(QString::number(processor.result->progressFailed)));
        }
        if (processor.result->progressUnprocessed > 0) {
            logText->append(tr("Files left unprocessed: %1").arg(QString::number(processor.result->progressUnprocessed)));
        }
    }

    if (processor.logfile) {
        logText->append(tr("For detailed report click "));

        QString filepath = processor.logfile->filePath();
        QTextCursor cursor = logText->textCursor();
        format.setAnchor(true);
        format.setAnchorHref(filepath);
        format.setFontUnderline(true);

        cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        cursor.insertText(tr("HERE"), format);
    }

    return;
}

void TeraMainWin::handleAbout() {
    AboutDialog *a = new AboutDialog(this);
    a->open();
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
        success = QProcess::startDetached("open", QStringList()  << link.url());
    }
#elif defined(Q_OS_WIN)
    if (!success) {
        success = QDesktopServices::openUrl(QUrl("file:///" + link.url(), QUrl::TolerantMode));
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

    qDebug() << "load " << appTranslator.load(":/translations/" + lang + ".qm");
    qDebug() << "load qt" << qtTranslator.load(":/translations/qtbase_" + lang + ".qm");

    retranslateUi(this);
    settingsWin.data()->retranslateUi(settingsWin.data());
    if (!filesWin.isNull()) {
        filesWin.data()->retranslateUi(filesWin.data());
    }
    if (!cardSelectDialog.isNull()) {
        cardSelectDialog.data()->retranslateUi(cardSelectDialog.data());
        cardSelectDialog.data()->onTranslate();
    }
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
    timestampingFinished(BatchStamper::FinishingDetails::cancelled(msg));
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
    handleSettingsFromPage(TeraSettingsWin::PAGE::__NONE);
}

void TeraMainWin::handleSettingsFromPage(TeraSettingsWin::PAGE openPage, bool openDirSelector) {
    processor.initializeSettingsWindow(*settingsWin);
    settingsWin->selectPage(openPage);
    settingsWin->open();
    if (openPage == TeraSettingsWin::PAGE::INPUT_DIR && openDirSelector) {
        settingsWin->openInclDirSearch();
    }
}

void TeraMainWin::handleSettingsAccepted() {
    processor.readSettings(*settingsWin);
    processor.saveSettings();
}

void TeraMainWin::processEvents() {
    QCoreApplication::processEvents();
}

void TeraMainWin::handleFilesAccepted() {
    // remember old list
    QSet<QString> oldList;
    QStringList l = processor.inFiles;
    for (int i = 0; i < l.size(); ++i)
        oldList.insert(l.at(i));
    l.clear();

    // update selected files list
    processor.copySelectedFiles(*filesWin);

    // remove selected ones to get to know which ones were excluded
    l = processor.inFiles;
    for (int i = 0; i < l.size(); ++i)
        oldList.remove(l.at(i));

    // log all de-selected files
    if (processor.logfile) {
        QList<QString> list = oldList.toList();
        qSort(list);
        for (int i = 0; i < list.size(); ++i) {
            processor.logfile->getStream() << "User de-selected: " << list.at(i) << endl;
        }
    }

    // continue with processing...
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
    int cancel_val = cancel.load();
    int jobid_val = jobId.load();
    return cancel_val != 0 || jobid != jobid_val;
}

}
