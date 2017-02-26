/*
 * gui_timestamper_processor.h
 *
 *  Created on: Nov 17, 2016
 */

#ifndef GUI_TIMESTAMPER_PROCESSOR_H_
#define GUI_TIMESTAMPER_PROCESSOR_H_

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QFileInfo>
#include <QScopedPointer>
#include <QSettings>
#include <QTextStream>

#ifndef TERA_USE_UNIX_STORAGE_INFO
    #include <QStorageInfo>
#endif

#include "config.h"
#include "logging.h"
#include "utils.h"

#include "files_window.h"
#include "settings_window.h"

namespace ria_tera {

class GuiTimestamperProcessor : public QObject {
    Q_OBJECT
public:
    class InFileData {
    public:
        InFileData(QString const& path) {
            QFileInfo fi(path);
            filesize = fi.size();
#ifdef TERA_USE_UNIX_STORAGE_INFO
            partitionPath = "/";
#else
            QStorageInfo si(path);
            partitionPath = si.rootPath();
#endif
        }
        qint64 filesize;
        QString partitionPath;
    };

    class Result {
    public:
        enum e_ProgressStage {
            TESTING_TIME_SERVER, SEARCHING_FILES, CONVERTING_FILES, DONE
        };
        Result() :
            progressStage(TESTING_TIME_SERVER), progressConverted(0),
            progressSuccess(0), progressFailed(0),
            success(false), cnt(-1), cntFound(-1) {}
        e_ProgressStage progressStage;
        int progressConverted;
        int progressSuccess;
        int progressFailed;
        bool success;
        QString error;
        int cnt;
        int cntFound;
    };

    GuiTimestamperProcessor();

    bool isShowIntroPage();
    void saveShowIntro(bool show);

    void processGlobalConfiguration();

    void initializeSettingsWindow(TeraSettingsWin& sw);
    void readSettings(TeraSettingsWin& sw);

    void readSettings();
    void saveSettings();

    void initializeFilePreviewWindow(FileListWindow& fw);
    void copySelectedFiles(FileListWindow& fw);

    bool openLogFile(QString& errorText);

    bool checkInDirListWithMessagebox(QWidget* parent);

    QString const INI_PARAM_PREVIEW_FILES;

    static bool checkInDirListWithMessagebox(QWidget* parent, QStringListModel const& inDirs);
    static bool checkInDirListWithMessagebox(QWidget* parent, QSet<QString> const& inDirs);

    static QString const& JSON_TERA_DEFAULT_OUT_EXT;
    static QString const& JSON_TERA_EXCL_DIRS_; // "TERA-EXCL-DIRS-"
public: // TODO
    Config config;
    QSettings settings; // register
    QString iniPath; // user changes

    bool showIntro;
    QString timeServerUrl;
    QString outExt;
    QSet<QString> centralExclDirs;
    QSet<QString> centralExclDirsDisabledByUser;
    QSet<QString> exclDirs;
    QSet<QString> inclDirs;
    bool previewFiles;

    QMap<QString, InFileData> foundFiles;
    QStringList inFiles;

    QScopedPointer<Result> result;
    QScopedPointer<LogFile> logfile;
};

}

#endif /* GUI_TIMESTAMPER_PROCESSOR_H_ */
