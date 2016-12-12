/*
 * gui_timestamper_processor.h
 *
 *  Created on: Nov 17, 2016
 */

#ifndef GUI_TIMESTAMPER_PROCESSOR_H_
#define GUI_TIMESTAMPER_PROCESSOR_H_

#include <QObject>

#include "config.h"
#include "utils.h"

#include "files_window.h"
#include "settings_window.h"

namespace ria_tera {

struct Result {
    Result() : success(false) {};
    bool success;
    QString error;
    int cnt;
};


class GuiTimestamperProcessor : public QObject {
    Q_OBJECT
public:
    GuiTimestamperProcessor();

    void initializeSettingsWindow(TeraSettingsWin& sw);
    void readSettings(TeraSettingsWin& sw);

    void initializeFilePreviewWindow(FileListWindow& fw);
    void copySelectedFiles(FileListWindow& fw);
public:
    Config config;

    QString timeServerUrl;
    QString outExt;
    QSet<QString> exclDirs;
    QSet<QString> inclDirs;
    bool previewFiles;

    QStringList inFiles;

    Result result;
};

}

#endif /* GUI_TIMESTAMPER_PROCESSOR_H_ */
