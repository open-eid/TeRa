/*
 * main_window.h
 *
 *  Created on: Nov 15, 2016
 *      Author: s
 */

#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QAtomicInt>
#include <QFileSystemModel>
#include <QWidget>

#include "ui_MainWindow.h"

#include "gui_timestamper_processor.h"
#include "files_window.h"
#include "timestamper.h"
#include "utils.h"

namespace ria_tera {

// http://stackoverflow.com/questions/26125363/how-to-add-checkbox-on-qtreeview-qfilesystemmodel
// TODO class FileSystemModel : public QFileSystemModel

class TeraMainWin;

class GuiProcessingMonitor : public ProcessingMonitorCallback
{
public:
    GuiProcessingMonitor(TeraMainWin& mainWindow);
    bool processingPath(QString const& path);
    bool excludingPath(QString const& path);
    bool foundFile(QString const& path);
    bool processingFile(QString const& pathIn, QString const& pathOut, int nr, int totalCnt);
private:
    TeraMainWin& gui;
};

class TeraMainWin: public QWidget, public Ui::MainWindow
{
    Q_OBJECT

public:
    explicit TeraMainWin(QWidget *parent = 0);
    ~TeraMainWin();
    void processEvents();
    bool isCancelled();
public slots:
    void handleStartStamping();
    void handleCancelProcess();

    void timestampingFinished(bool success, QString errString);

    void handleSettings();
    void handleSettingsAccepted();

    void handleFilesAccepted();
    void handleFilesRejected();
private:
    void startStampingFiles(); // TODO better name
private:
    QAtomicInt cancel;
    GuiTimestamperProcessor processor;
    GuiProcessingMonitor monitor;
    OutputNameGenerator nameGen;
    BatchStamper stamper;

    TeraSettingsWin* settingsWin;
    FileListWindow* filesWin;
};

}

#endif /* MAIN_WINDOW_H_ */
