/*
 * gui_timestamper_processor.h
 *
 *  Created on: Nov 17, 2016
 */

#ifndef GUI_TIMESTAMPER_PROCESSOR_H_
#define GUI_TIMESTAMPER_PROCESSOR_H_

#include <QObject>
#include <QFile>
#include <QScopedPointer>
#include <QTextStream>

#include "config.h"
#include "utils.h"

#include "files_window.h"
#include "settings_window.h"

namespace ria_tera {

class GuiTimestamperProcessor : public QObject {
    Q_OBJECT
public:
    class Result {
    public:
        enum e_ProgressStage {
            TESTING_TIME_SERVER, SEARCHING_FILES, CONVERTING_FILES, DONE
        };
        Result() :
            progressStage(TESTING_TIME_SERVER), progressConverted(0),
            progressSuccess(0), progressFailed(0),
            success(false), cnt(-1) {}
        e_ProgressStage progressStage;
        int progressConverted;
        int progressSuccess;
        int progressFailed;
        bool success;
        QString error;
        int cnt;
    };

    class LogFile {
    public:
        LogFile(QFile* file);
        virtual ~LogFile();
        QString filePath();
        QTextStream& getStream() { return logStream; };
        void close();
    private:
        QScopedPointer<QFile> logFile;
        QTextStream logStream;
    };

    GuiTimestamperProcessor();

    void initializeSettingsWindow(TeraSettingsWin& sw);
    void readSettings(TeraSettingsWin& sw);

    void initializeFilePreviewWindow(FileListWindow& fw);
    void copySelectedFiles(FileListWindow& fw);

    bool openLogFile(QString& errorText);
public:
    Config config;

    QString timeServerUrl;
    QString outExt;
    QSet<QString> exclDirs;
    QSet<QString> inclDirs;
    bool previewFiles;

    QSet<QString> foundFiles;
    QStringList inFiles;

    QScopedPointer<Result> result;
    QScopedPointer<LogFile> logfile;
};

}

#endif /* GUI_TIMESTAMPER_PROCESSOR_H_ */
