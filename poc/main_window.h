/*
 * main_window.h
 *
 *  Created on: Nov 15, 2016
 *      Author: s
 */

#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QAction>
#include <QAtomicInt>
#include <QAtomicPointer>
#include <QFileSystemModel>
#include <QRunnable>
#include <QTranslator>
#include <QWidget>

#include "ui_MainWindow.h"

#include "disk_crawler.h"
#include "files_window.h"
#include "gui_timestamper_processor.h"
#include "timestamper.h"
#include "utils.h"

namespace ria_tera {

// http://stackoverflow.com/questions/26125363/how-to-add-checkbox-on-qtreeview-qfilesystemmodel
// TODO class FileSystemModel : public QFileSystemModel

class TeraMainWin;

class CrawlDiskJob : public QObject, public QRunnable, public DiscCrawlMonitorCallback
{
    Q_OBJECT

public:
    CrawlDiskJob(TeraMainWin& mainWindow, int jobid, GuiTimestamperProcessor const & processor);
    virtual void run();

    virtual bool processingPath(QString const& path, double progress_percent);
    virtual bool excludingPath(QString const& path);
    virtual bool foundFile(QString const& path);
signals:
    void signalProcessingPath(int jobid, QString path, double progress_percent);
    void signalExcludingPath(int jobid, QString path);
    void signalFoundFile(int jobid, QString path);
    void signalFindingFilesDone(int jobid);
private:
    bool isCanceled();

    TeraMainWin& gui;
    int jobId;
    DiskCrawler dc;
};

class TeraMainWin : public QWidget, public Ui::MainWindow, public StampingMonitorCallback
{
    Q_OBJECT

public:
    explicit TeraMainWin(QWidget *parent = 0);
    ~TeraMainWin();
    void processEvents();
    bool isCancelled();
    bool isCancelled(int jobid);
public slots:
    // flow control slots
    void handleStartStamping();
    void handleCancelProcess();
    void handleReadyButton();

    void timestampingTestFinished(bool success, QByteArray resp, QString errString);

    void processProcessingPath(int jobid, QString path, double progress_percent);
    void processExcludingPath(int jobid, QString path); // TODO delete
    void processFoundFile(int jobid, QString path);
    void processFindingFilesDone(int jobid);

    void doFindingFilesDone();
    void startStampingFiles(); // TODO better name
private:
    bool processingFile(QString const& pathIn, QString const& pathOut, int nr, int totalCnt);
    bool processingFileDone(QString const& pathIn, QString const& pathOut, int nr, int totalCnt, bool success, QString const& errString);
public slots:
    void timestampingFinished(bool success, QString errString);

    // GUI related slots
    void handleSettings();
    void handleSettingsAccepted();

    void handleFilesAccepted();
    void handleFilesRejected();

    void slotLanguageChanged(int i);
    void slotLanguageChanged(QAction* action);

    void showLog(QUrl const& link);
protected:
    virtual void closeEvent(QCloseEvent *event);
    virtual void changeEvent(QEvent *event);

    void fillProgressBar();
    void fillDoneLog();
private:
    void doUserCancel();
    void loadTranslation(QString const& language_short);
private:
    bool timestapmping;
    QAtomicInt cancel;
    QAtomicInt jobId;

    GuiTimestamperProcessor processor;
    OutputNameGenerator nameGen;
    BatchStamper stamper;

    TeraSettingsWin* settingsWin;
    FileListWindow* filesWin;

    QString lang;
    QStringList langs;
    QTranslator appTranslator;
};

}

#endif /* MAIN_WINDOW_H_ */
