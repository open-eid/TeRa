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

#include "id_card_select_dialog.h"
#include "../src/common/HttpsIDCardAuthentication.h"

namespace ria_tera {

// http://stackoverflow.com/questions/26125363/how-to-add-checkbox-on-qtreeview-qfilesystemmodel
// TODO class FileSystemModel : public QFileSystemModel

class TeraMainWin;

class CrawlDiskJob : public QObject, public QRunnable, public DiscCrawlMonitorCallback
{
    Q_OBJECT

public:
    CrawlDiskJob(TeraMainWin& mainWindow, int jobid, GuiTimestamperProcessor const & processor, QStringList const &extensions);
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

    void doPin1Authentication();
    void pin1AuthenticaionDone();
    void doTestStamp();
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
    void timestampingFinished(BatchStamper::FinishingDetails details);

    void globalConfFinished(bool changed, const QString &error);
    void globalConfNetworkError(const QString &error);

    // GUI related slots
    void introAccept();
    void introReject();

    void handleSettings();
    void handleSettingsFromPage(TeraSettingsWin::PAGE openPage, bool openDirSelector = false);
    void handleSettingsAccepted();

    void handleFilesAccepted();
    void handleFilesRejected();

    void slotLanguageChanged(int i);
    void slotLanguageChanged(QAction* action);

    void handleAbout();
    void handleHelp();

    void showLog(QUrl const& link);
protected:
    virtual void closeEvent(QCloseEvent *event);
    virtual void changeEvent(QEvent *event);

    /// Check if settings are complete + do required GUI interactions
    bool checkSettingsWithGUI();
    void fillProgressBar();
    void fillDoneLog();
private:
    enum PAGE {START, PROCESS, READY, INTRO};
    void resetLogFormat();
    void setPage(PAGE p);
    void setBackgroundImg(QString path);
    void doUserCancel(QString msg = QString());
    void loadTranslation(QString const& language_short);
#ifdef Q_OS_OSX
    bool grantPermissions(QSet<QString> const& deniedDirs);
#endif
private:
    bool initDone;
    bool showingIntro;
    bool timestapmping;
    QAtomicInt cancel;
    QAtomicInt jobId;

    GuiTimestamperProcessor processor;
    OutputNameGenerator nameGen;
    BatchStamper stamper;

    QSharedPointer<TeraSettingsWin> settingsWin;
    QSharedPointer<FileListWindow> filesWin;
    QSharedPointer<IDCardSelectDialog> cardSelectDialog;
    HttpsIDCardAuthentication idCardAuth;

    QString lang;
    QStringList langs;
    QTranslator appTranslator, qtTranslator;

    QPushButton* btnIntroAccept;
    QPushButton* btnIntroReject;

    QString backgroundImg;

    QStringList selectedExtensions;
};

}

#endif /* MAIN_WINDOW_H_ */
