/*
 * main_window.cpp
 *
 *  Created on: Nov 15, 2016
 */

#include "main_window.h"

#include <iostream>

#include <QCloseEvent>
#include <QDirIterator>
#include <QFile>
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
    QWidget(parent), monitor(*this),
    nameGen("ddoc", "asics"), // TODO
    stamper(monitor, nameGen),
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
    processor.inFiles.clear();
    timestapmping = true;

    stackedCntrlWidget->setCurrentIndex(1); // TODO
    settings->setEnabled(false);
    progressBar->setMaximum(PP_TS_TEST + PP_SEARCH + PP_TS);
    progressBar->setValue(0);
    progressBar->setFormat(tr("Testing Time Server..."));
    //

    logText->clear();
    logText->append(tr("Starting..."));

    logText->append(tr("Testing Time Server..."));

    QByteArray pseudosha256(256/8, '\0');
    QByteArray req = stamper.getTimestamper().getTimestampRequest4Sha256(pseudosha256);
    stamper.getTimestamper().timeserverUrl = processor.timeServerUrl; // TODO
    stamper.getTimestamper().sendTSRequest(req, true); // TODO api is rubbish
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

void TeraMainWin::timestampingTestFinished(bool success, QByteArray resp, QString errString) {
    if (!success) {
        timestampingFinished(false, tr("Test request to Time Server failed. ") + errString);
        return;
    }
    // TODO success for TS test?

    if (isCancelled()) {
        doUserCancel();
        return;
    }

    progressBar->setValue(PP_TS_TEST);
    progressBar->setFormat(tr("Testing Time Server..."));

    CrawlDiskJob* crawlJob = new CrawlDiskJob(*this, ++jobId, processor);
    connect(crawlJob, SIGNAL(signalProcessingPath(int, QString, double)),
        this, SLOT(processProcessingPath(int, QString, double)));
    connect(crawlJob, SIGNAL(signalExcludingPath(int, QString)),
        this, SLOT(processExcludingPath(int, QString)));
    connect(crawlJob, SIGNAL(signalFoundFile(int, QString)),
        this, SLOT(processFoundFile(int, QString)));
    connect(crawlJob, SIGNAL(signalFindingFilesDone(int)),
        this, SLOT(processFindingFilesDone(int)));

    processor.inFiles.clear();
    QThreadPool::globalInstance()->start(crawlJob);
}

void TeraMainWin::processProcessingPath(int jobid, QString path, double progress_percent) {
    if (isCancelled(jobid)) return;
    logText->append(QObject::tr("Searching") + " " + path); // TODO
    progressBar->setValue((int)(PP_TS_TEST + progress_percent * PP_SEARCH));
    progressBar->setFormat(QObject::tr("Searching") + " " + path + "...");
}

void TeraMainWin::processExcludingPath(int jobid, QString path) {
    qDebug() << "processExcludingPath " << path; // TODO delete ?????????????????
}

void TeraMainWin::processFoundFile(int jobid, QString path) {
qDebug() << " xxx" << path; // TODO delete ?????????????????
    if (isCancelled(jobid)) return;
    processor.inFiles.append(path);
}

void TeraMainWin::processFindingFilesDone(int jobid) {
    qDebug() << " o";
    if (isCancelled(jobid)) return;
    doFindingFilesDone();
}

void TeraMainWin::doFindingFilesDone() {
    if (isCancelled()) {
        doUserCancel();
        return;
    }

    if (processor.inFiles.size() > 0 && processor.previewFiles) {
        processor.initializeFilePreviewWindow(*filesWin);
        filesWin->open();
    } else {
        startStampingFiles();
    }
}

void TeraMainWin::startStampingFiles() {
    stamper.startTimestamping(processor.timeServerUrl, processor.inFiles);
}

void TeraMainWin::timestampingFinished(bool success, QString errString) {
    if (success) {
        processor.result.success = true;
        processor.result.cnt = processor.inFiles.size(); // TODO
    }
    else {
        processor.result.success = false;
        processor.result.error = errString;
        logText->clear();
    }
    fillDoneLog();

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
        fillDoneLog();
    }
}

void TeraMainWin::fillDoneLog() {
    logText->clear();
    if (processor.result.success) {
        QString totalCnt = QString::number(processor.result.cnt);
        logText->append(tr("DDOC failide konverteerimine lõppes edukalt"));
        logText->append(tr("DDOC faile leitud: ") + totalCnt);
        logText->append(tr("DDOC faile konverteeritud: ") + totalCnt);
        logText->append(tr("Täpsema aruande vaatamiseks vajuta ") + "TODO");
    }
    else {
        logText->append(tr("Error: ") + processor.result.error);
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

void TeraMainWin::doUserCancel() {
    cancel = true;
    timestampingFinished(false, tr("Operation cancelled by user..."));
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

GuiProcessingMonitor::GuiProcessingMonitor(TeraMainWin& mainWindow) : gui(mainWindow) {}

bool GuiProcessingMonitor::processingPath(QString const& path, double progress_percent) {
gui.logText->append(QObject::tr("Searching") + " " + path); // TODO
gui.progressBar->setValue((int)(PP_TS_TEST + progress_percent * PP_SEARCH));
gui.progressBar->setFormat(QObject::tr("Searching") + " " + path + "...");
    gui.processEvents();
    return !gui.isCancelled();
}

bool GuiProcessingMonitor::excludingPath(QString const& path) {
// TODO    gui.logText->append("   " + QObject::tr("Excluding") + " " + path);
    gui.processEvents();
    return !gui.isCancelled();
}

bool GuiProcessingMonitor::foundFile(QString const& path) { // TODO delete

// TODO    gui.logText->append("   " + QObject::tr("Found") + " " + path);
    gui.processEvents();
    return !gui.isCancelled();
}

bool GuiProcessingMonitor::processingFile(QString const& pathIn, QString const& pathOut, int nr, int totalCnt) {
// TODO    gui.logText->append(QObject::tr("Timestamping") + " (" + QString::number(nr+1) + "/" + QString::number(totalCnt) + ") " + pathIn + " -> " + pathOut);
gui.progressBar->setValue((int)(PP_TS_TEST + PP_SEARCH + (double) nr / totalCnt * PP_TS));
gui.progressBar->setFormat(QObject::tr("Timestamping") + " (" + QString::number(nr + 1) + "/" + QString::number(totalCnt) + ") " + pathIn /*+ " -> " + pathOut*/);
    gui.processEvents();
    return !gui.isCancelled();
}

}
