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
    settingsWin(NULL),
    appTranslator(this)
{
    setupUi(this);
    setStyleSheet("background-image: url(:/images/background.png);");
    settings->setStyleSheet("QPushButton:disabled"
            "{ color: gray }");

    connect(btnStamp, SIGNAL (clicked()), this, SLOT (handleStartStamping()), Qt::QueuedConnection);
    connect(settings, SIGNAL (clicked()), this, SLOT (handleSettings()), Qt::QueuedConnection);
    connect(cancelProcess, SIGNAL (clicked()), this, SLOT (handleCancelProcess()), Qt::QueuedConnection);

    connect(&stamper, SIGNAL(timestampingFinished(bool,QString)),
            this, SLOT(timestampingFinished(bool,QString)), Qt::QueuedConnection);
    connect(&stamper.getTimestamper(), SIGNAL(timestampingTestFinished(bool,QByteArray,QString)),
            this, SLOT(timestampingTestFinished(bool,QByteArray,QString)), Qt::QueuedConnection);

    settingsWin = new TeraSettingsWin(this);
    connect(settingsWin, SIGNAL(accepted()),
            this, SLOT(handleSettingsAccepted()), Qt::QueuedConnection);

    filesWin = new FileListWindow(this);
    connect(filesWin, SIGNAL(accepted()),
            this, SLOT(handleFilesAccepted()), Qt::QueuedConnection);
    connect(filesWin, SIGNAL(rejected()),
            this, SLOT(handleFilesRejected()), Qt::QueuedConnection);

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
}

TeraMainWin::~TeraMainWin()
{
}

QString doFileList(QList<QString> list) {
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
        QString dirlist = doFileList(doesntExist);
        QMessageBox::critical(this, tr("Error"), tr("The following input folders don't exist (or are files). Cancelling:") + "\n" + dirlist);
        return;
    }

    cancel = false;
    stackedCntrlWidget->setCurrentIndex(1); // TODO
    settings->setEnabled(false);

    logText->clear();
    logText->append(tr("Starting..."));

    logText->append(tr("Testing Time Server..."));

    QByteArray pseudosha256(256/8, '\0');
    QByteArray req = stamper.getTimestamper().getTimestampRequest4Sha256(pseudosha256);
    stamper.getTimestamper().timeserverUrl = processor.timeServerUrl; // TODO
    stamper.getTimestamper().sendTSRequest(req, true); // TODO api is rubbish
    //timestampingTestFinished(true, QByteArray(), ""); // TODO
}

void TeraMainWin::timestampingTestFinished(bool success, QByteArray resp, QString errString) {
    if (!success) {
        timestampingFinished(false, tr("Test request to Time Server failed. ") + errString);
        return;
    }
    // TODO success for TS test?

    if (isCancelled()) { // TODO code copy
        timestampingFinished(false, tr("Operation cancelled by user..."));
        return;
    }

    QString in_extension(Config::EXTENSION_IN);
    ria_tera::DiskCrawler dc(monitor, in_extension);
    dc.addExcludeDirs(processor.exclDirs.toList());

    QList<QString> inDirList = processor.inclDirs.toList(); // TODO checked previously
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
    cancel = true;
}

void TeraMainWin::handleSettings() {
    processor.initializeSettingsWindow(*settingsWin);
    settingsWin->open();
}

void TeraMainWin::handleSettingsAccepted() {
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

    stackedCntrlWidget->setCurrentIndex(0);
    settings->setEnabled(true);
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

void TeraMainWin::slotLanguageChanged(int i) {
    loadTranslation( langs[i] ); // TODO check range
}

void TeraMainWin::slotLanguageChanged(QAction* a) {
    loadTranslation( a->data().toString() );
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
