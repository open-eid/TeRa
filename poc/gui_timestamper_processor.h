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

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
#else
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
#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
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
        e_ProgressStage progressStage = TESTING_TIME_SERVER;
        int progressConverted = 0;
        int progressSuccess = 0;
        int progressFailed = 0;
        int progressUnprocessed = 0;
        bool success = false;
        QString error;
        /// show "Error:" before error string?
        bool isSystemError = true;
        int cnt = -1;
        int cntFound = -1;
    };

    GuiTimestamperProcessor();

    bool isShowIntroPage();
    void saveShowIntro(bool show);

    void processGlobalConfiguration();

    void initializeSettingsWindow(TeraSettingsWin& sw);
    void readSettings(TeraSettingsWin& sw);

    void readSettings();
    void saveSettings();

    QList<QString> getInclDirList() const;
#ifdef Q_OS_OSX
    void resetGrants(QScopedPointer<QSet<QString>> &removed, QScopedPointer<QSet<QString>> &added);
#endif

    void initializeFilePreviewWindow(FileListWindow& fw);
    void copySelectedFiles(FileListWindow& fw);

    bool openLogFile(QString& errorText);

    bool checkInDirListWithMessagebox(QWidget* parent);

    QString const INI_PARAM_PREVIEW_FILES;

    static bool checkInDirListWithMessagebox(QWidget* parent, QStringListModel const& inDirs);
    static bool checkInDirListWithMessagebox(QWidget* parent, QSet<QString> const& inDirs);

    static QString const JSON_TERA_MIN_SUPPORTED_VERSION;
    static QString const JSON_TERA_DEFAULT_OUT_EXT;
    static QString const JSON_TERA_EXCL_DIRS_; // "TERA-EXCL-DIRS-"
public: // TODO
    Config config;
    QSettings settings; // register
    QString iniPath; // user changes

    bool showIntro;
    bool stampDDoc;
    bool stampBDoc;

    QString minSupportedVersion;
    QString timeServerUrl;
    QString outExt;
    QSet<QString> centralExclDirs;
    QSet<QString> centralExclDirsDisabledByUser;
    QSet<QString> exclDirs;
private:
    /// For lazy evaluation of inclDirs
    QSet<QString>& _getInclDirs();
    QSet<QString> const& _getInclDirs() const;
    void syncSettings();
    mutable QScopedPointer<QSet<QString>> inclDirs;
#ifdef Q_OS_OSX
    QScopedPointer<QSet<QString>> revoked;
    QScopedPointer<QSet<QString>> granted;
#endif
public:
    bool previewFiles;

    QMap<QString, InFileData> foundFiles;
    QStringList inFiles;

    QScopedPointer<Result> result;
    /// raport/logfile
    QScopedPointer<LogFile> logfile;
};

}

#endif /* GUI_TIMESTAMPER_PROCESSOR_H_ */
