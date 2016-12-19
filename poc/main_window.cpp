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
#include <QThreadPool>

#include "disk_crawler.h"
#include "settings_window.h"

namespace {
    static int PP_TS_TEST = 100;
    static int PP_SEARCH = 500;
    static int PP_TS = 400;
}

namespace ria_tera {

TeraMainWin::TeraMainWin(QWidget *parent) :
    QWidget(parent),
    nameGen("ddoc", "asics"), // TODO consts
    stamper(*this, nameGen, false),
    settingsWin(NULL),
    appTranslator(this)
{
    setupUi(this);

    setStyleSheet("background-image: url(:/images/background.png);");
    settings->setStyleSheet("QPushButton:disabled"
            "{ color: gray }");

    stackedCntrlWidget->setCurrentIndex(0);

    connect(btnStamp, SIGNAL (clicked()), this, SLOT (handleStartStamping()));
    connect(settings, SIGNAL (clicked()), this, SLOT (handleSettings()));
    connect(cancelProcess, SIGNAL(clicked()), this, SLOT(handleCancelProcess()));
    connect(btnReady, SIGNAL(clicked()), this, SLOT(handleReadyButton()));

    connect(&stamper, SIGNAL(timestampingFinished(bool,QString)),
            this, SLOT(timestampingFinished(bool,QString)));
    connect(&stamper.getTimestamper(), SIGNAL(timestampingTestFinished(bool,QByteArray,QString)),
            this, SLOT(timestampingTestFinished(bool,QByteArray,QString)));

    connect(logText, SIGNAL(anchorClicked(const QUrl&)),
            this, SLOT(showLog(QUrl const&)));

    settingsWin = new TeraSettingsWin(this);
    connect(settingsWin, SIGNAL(accepted()),
            this, SLOT(handleSettingsAccepted()));

    filesWin = new FileListWindow(this);
    connect(filesWin, SIGNAL(accepted()),
            this, SLOT(handleFilesAccepted()));
    connect(filesWin, SIGNAL(rejected()),
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
}

TeraMainWin::~TeraMainWin()
{
}

QString toBulletedList(QList<QString> list) { // TODO rename
    QString res;
    for (int i = 0; i < list.size(); ++i) {
        if (0 == i) res = " * " + list[i];
        else res += "\n * " + list[i];
    }
    return res;
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

void TeraMainWin::handleStartStamping() {
    if (0 == processor.inclDirs.size()) {
        QMessageBox::critical(this, tr("Error"), tr("No input directory selected."));
        return;
    }

    processor.timeServerUrl = processor.timeServerUrl.trimmed(); // TODO
    if (processor.timeServerUrl.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Time server URL is empty."));
        return;
    }

    QList<QString> inDirList = processor.inclDirs.toList(); // TODO make checking better
    QList<QString> doesntExist;
    for (int i = 0; i < inDirList.size(); ++i) {
        QString dir = inDirList.at(i);
        QFileInfo fi(dir);

        if (!fi.exists() || !fi.isDir()) {
            doesntExist.append(dir);
        }
    }

    if (!doesntExist.empty()) {
        QString dirlist = toBulletedList(doesntExist);
        QMessageBox::critical(this, tr("Error"), tr("The following input folders don't exist (or are files). Cancelling:") + "\n" + dirlist);
        return;
    }

    //
    cancel = false;
    processor.foundFiles.clear();
    processor.inFiles.clear();
    timestapmping = true;

    // create file
    QString error;
    processor.result.reset(new GuiTimestamperProcessor::Result());
    if (!processor.openLogFile(error)) { // TODO  finish message box
        QMessageBox mb(this);
        mb.exec();
    }

    stackedCntrlWidget->setCurrentIndex(1); // TODO
    settings->setEnabled(false);
    progressBar->setMaximum(PP_TS_TEST + PP_SEARCH + PP_TS);
    progressBar->setValue(0);
    fillProgressBar();

    QByteArray pseudosha256(256/8, '\0');
    QByteArray req = stamper.getTimestamper().getTimestampRequest4Sha256(pseudosha256);
    stamper.getTimestamper().timeserverUrl = processor.timeServerUrl; // TODO
    stamper.getTimestamper().sendTSRequest(req, true); // TODO api is rubbish
}

void TeraMainWin::timestampingTestFinished(bool success, QByteArray resp, QString errString) {
    if (!success) {
        timestampingFinished(false, tr("Test request to Time Server failed. ") + errString);
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

    CrawlDiskJob* crawlJob = new CrawlDiskJob(*this, ++jobId, processor);
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
                qDebug() << "X " << filePath << " " << data.partitionPath << " " << data.filesize;
            }
        }

        bool spaceIssue = false;
        QString sizeInfo;
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
    stackedCntrlWidget->setCurrentIndex(2);
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
    logText->append(tr("DDOC faile leitud: %1").arg(QString::number(processor.result->cnt)));
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

void TeraMainWin::showLog(QUrl const& link) {
    if (!QDesktopServices::openUrl(link)) {
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
    stackedCntrlWidget->setCurrentIndex(0);
}

void TeraMainWin::handleSettings() {
    processor.initializeSettingsWindow(*settingsWin);
    settingsWin->open();
}

void TeraMainWin::handleSettingsAccepted() {
    processor.readSettings(*settingsWin);
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
    return cancel != 0;
}

bool TeraMainWin::isCancelled(int jobid) {
    return cancel != 0 || jobid != jobId;
}

}
