/*
 * main_window.cpp
 *
 *  Created on: Nov 15, 2016
 */

#include "main_window.h"

#include <iostream>

#include <QDirIterator>
#include <QFile>
#include <QMessageBox>

#include "disk_crawler.h"
#include "settings_window.h"

namespace ria_tera {

TeraMainWin::TeraMainWin(QWidget *parent) :
    QWidget(parent), monitor(*this),
    nameGen("ddoc", "asics"), // TODO
    stamper(monitor, nameGen),
    settingsWin(NULL)
{
    setupUi(this);
    setStyleSheet("background-image: url(:/images/background.png);");
    cancelProcess->setEnabled(false);

    connect(btnStamp, SIGNAL (clicked()), this, SLOT (handleStartStamping()), Qt::QueuedConnection);
    connect(settings, SIGNAL (clicked()), this, SLOT (handleSettings()), Qt::QueuedConnection);
    connect(cancelProcess, SIGNAL (clicked()), this, SLOT (handleCancelProcess()), Qt::QueuedConnection);

    connect(&stamper, SIGNAL(timestampingFinished(bool,QString)),
            this, SLOT(timestampingFinished(bool,QString)), Qt::QueuedConnection);

    settingsWin = new TeraSettingsWin(this);
    connect(settingsWin, SIGNAL(accepted()),
            this, SLOT(handleSettingsAccepted()), Qt::QueuedConnection);

    filesWin = new FileListWindow(this);
    connect(filesWin, SIGNAL(accepted()),
            this, SLOT(handleFilesAccepted()), Qt::QueuedConnection);
    connect(filesWin, SIGNAL(rejected()),
            this, SLOT(handleFilesRejected()), Qt::QueuedConnection);
}

TeraMainWin::~TeraMainWin()
{
}

void TeraMainWin::handleStartStamping() {
    if (0 == processor.inclDirs.size()) {
        QMessageBox::critical(this, tr("XXXX"), tr("No input directory selected."));
        return;
    }

    if (processor.timeServerUrl.trimmed().isEmpty()) {
        QMessageBox::critical(this, tr("XXXX"), tr("Time server URL is empty."));
        return;
    }

    cancel = false;
    btnStamp->setEnabled(false); // TODO
    cancelProcess->setEnabled(true);

    logText->clear();
    logText->append(tr("Starting..."));

    ; // TODO test connection

    QString in_extension(Config::EXTENSION_IN);
    ria_tera::DiskCrawler dc(monitor, in_extension);
    dc.addExcludeDirs(processor.exclDirs.toList());

    QList<QString> inDirList = processor.inclDirs.toList();
    for (int i = 0; i < inDirList.size(); ++i) {
        QString dir = inDirList.at(i);
        dc.addInputDir(dir, true);
    }

    QStringList inFiles;
    inFiles = dc.crawl();

    if (isCancelled()) {
        timestampingFinished(false, tr("Operation cancelled by user..."));
        return;
    }

    if (inFiles.size() > 0 && processor.previewFiles) {
        processor.inFiles = inFiles;
        processor.initializeFilePreviewWindow(*filesWin);
        filesWin->open();
    } else {
        processor.inFiles = inFiles;
        startStampingFiles();
    }
}

void TeraMainWin::startStampingFiles() {
    stamper.startTimestamping(processor.timeServerUrl, processor.inFiles);
}

void TeraMainWin::handleCancelProcess() {
    cancel = true;
}

void TeraMainWin::handleSettings() {
    processor.initializeSettingsWindow(*settingsWin);
    settingsWin->open();
}

void TeraMainWin::handleSettingsAccepted() {
std::cout << "TeraMainWin::handleSettingsAccepted" << std::endl;
    processor.readSettings(*settingsWin);
}

void TeraMainWin::timestampingFinished(bool success, QString errString) {
    logText->append("");

    if (success) {
        QString totalCnt = QString::number(processor.inFiles.size());
        logText->append(QString(tr("Processed %1 files")).arg(totalCnt));
        logText->append("");

        logText->append(tr("Timestamping finished successfully :)"));
    } else {
        logText->append(tr("Error: ") + errString);
    }

    cancelProcess->setEnabled(false);
    btnStamp->setEnabled(true);
}

void TeraMainWin::processEvents() {
    QCoreApplication::processEvents();
}

void TeraMainWin::handleFilesAccepted() {
    processor.copySelectedFiles(*filesWin);
    startStampingFiles();
}

void TeraMainWin::handleFilesRejected() {
    timestampingFinished(false, tr("Operation cancelled by user...")); // TODO
}

bool TeraMainWin::isCancelled() {
    return cancel != 0;
}

GuiProcessingMonitor::GuiProcessingMonitor(TeraMainWin& mainWindow) : gui(mainWindow) {}

bool GuiProcessingMonitor::processingPath(QString const& path) {
    gui.logText->append(QObject::tr("Searching") + " " + path);
    gui.processEvents();
    return !gui.isCancelled();
}

bool GuiProcessingMonitor::excludingPath(QString const& path) {
    gui.logText->append("   " + QObject::tr("Excluding") + " " + path);
    gui.processEvents();
    return !gui.isCancelled();
}

bool GuiProcessingMonitor::foundFile(QString const& path) {
    gui.logText->append("   " + QObject::tr("Found") + " " + path);
    gui.processEvents();
    return !gui.isCancelled();
}

bool GuiProcessingMonitor::processingFile(QString const& pathIn, QString const& pathOut, int nr, int totalCnt) {
    gui.logText->append(QObject::tr("Timestamping") + " (" + QString::number(nr+1) + "/" + QString::number(totalCnt) + ") " + pathIn + " -> " + pathOut);
    gui.processEvents();
    return !gui.isCancelled();
}

}
