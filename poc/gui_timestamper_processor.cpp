/*
 * gui_timestamper_processor.cpp
 *
 *  Created on: Nov 17, 2016
 */

#include "gui_timestamper_processor.h"

#include <iostream>

#include <QtAlgorithms>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QJsonObject>
#include <QMessageBox>
#include <QStringListModel>
#include <QStandardPaths>
#include <QTemporaryFile>

#include "utils.h"
#include "../src/libdigidoc/Configuration.h"

namespace {

QString toBulletedList(QList<QString> list) {
    QString res;
    for (int i = 0; i < list.size(); ++i) {
        if (0 == i) res = " * " + list[i];
        else res += "\n * " + list[i];
    }
    return res;
}

}

namespace ria_tera {

static char const* REG_PARAM_SHOW_INTRO = "ShowIntro";
QString const GuiTimestamperProcessor::JSON_TERA_MIN_SUPPORTED_VERSION("TERA-SUPPORTED");
QString const GuiTimestamperProcessor::JSON_TERA_DEFAULT_OUT_EXT("TERA-DEFAULT-OUT-EXTENSION");
QString const GuiTimestamperProcessor::JSON_TERA_EXCL_DIRS_("TERA-EXCL-DIRS-");

GuiTimestamperProcessor::GuiTimestamperProcessor() :
    INI_PARAM_PREVIEW_FILES(Config::INI_GROUP_ + "preview_files"),
    settings("Estonian ID Card", qApp->applicationName())
{
#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
    QDir iniDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
#else
    QDir iniDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
#endif
    iniDir.mkpath(iniDir.path());
    iniPath = QFileInfo(iniDir, "tera_client.ini").filePath();

    config.appendIniFile(iniPath);
    readSettings();
}

bool GuiTimestamperProcessor::isShowIntroPage() {
    return showIntro;
}

void GuiTimestamperProcessor::saveShowIntro(bool show) {
    showIntro = show;
    settings.setValue(REG_PARAM_SHOW_INTRO, QVariant(showIntro));
    settings.sync();
}

void QSet2GUI(QSet<QString> const& set, QStringListModel& model) {
    QList<QString> l = set.toList();
    qSort(l);
    model.setStringList(l);
}

void GuiTimestamperProcessor::processGlobalConfiguration() {
    QJsonObject o = Configuration::instance().object();
    QString excl = o.value(JSON_TERA_EXCL_DIRS_ + OS_SHORT).toString();
    centralExclDirs.clear();
    Config::append_excl_dirs(excl, centralExclDirs);

    QSet<QString> toBeAdded(centralExclDirs);
    toBeAdded.subtract(centralExclDirsDisabledByUser);

    exclDirs.unite(toBeAdded);

    QString jsonOutExt = o.value(JSON_TERA_DEFAULT_OUT_EXT).toString();

    if (!jsonOutExt.isNull()) {
        outExt = jsonOutExt;
    }

    // min supported version
    minSupportedVersion = o.value(JSON_TERA_MIN_SUPPORTED_VERSION).toString();
}

void GuiTimestamperProcessor::initializeSettingsWindow(TeraSettingsWin& sw) {
    while (sw.tabWidget->count() > 3) {
        sw.tabWidget->removeTab(3);
    }

    // show intro flag
    sw.cbShowIntro->setChecked(showIntro);

    // time server url
    sw.lineTSURL->setText(timeServerUrl);

    // exclude dirs
    QSet2GUI(exclDirs, *sw.modelExclDir);

    // include dirs
    QSet2GUI(_getInclDirs(), *sw.modelInclDir);

    // preview files
    sw.cbPreviewFiles->setChecked(previewFiles);
}

void GUI2QSet(QStringListModel const& model, QSet<QString>& set) {
    set = QSet<QString>::fromList(model.stringList());
}

void GuiTimestamperProcessor::readSettings(TeraSettingsWin& sw) {
    timeServerUrl = sw.lineTSURL->text().trimmed();

    GUI2QSet(*sw.modelExclDir, exclDirs);
    GUI2QSet(*sw.modelInclDir, _getInclDirs());

    previewFiles = sw.cbPreviewFiles->isChecked();

    showIntro = sw.cbShowIntro->isChecked();
}

void GuiTimestamperProcessor::initializeFilePreviewWindow(FileListWindow& fw) {
    fw.setFileList(inFiles);
}

void GuiTimestamperProcessor::copySelectedFiles(FileListWindow& fw) {
    inFiles = fw.extractSelectedFileList();
}

bool GuiTimestamperProcessor::openLogFile(QString& errorText) {
    logfile.reset();

    QDir dir = QDir::home();
    QString file_prefix = "tera_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz");
    QString file_sufix = ".log";

    QString error;
    LogFile* log = LogFile::openLogFile(dir, file_prefix, file_sufix, error);

    if (log) {
        logfile.reset(log);
        return true;
    } else {
        errorText = error;
        return false;
    }
}

bool GuiTimestamperProcessor::checkInDirListWithMessagebox(QWidget* parent) {
    return checkInDirListWithMessagebox(parent, _getInclDirs());
}

bool GuiTimestamperProcessor::checkInDirListWithMessagebox(QWidget* parent, QStringListModel const& inDirs) {
    QSet<QString> ids;
    GUI2QSet(inDirs, ids);
    return checkInDirListWithMessagebox(parent, ids);
}

bool GuiTimestamperProcessor::checkInDirListWithMessagebox(QWidget* parent, QSet<QString> const& inDirs) {
    QList<QString> inDirList = inDirs.toList(); // TODO make checking better

    if (0 == inDirs.size()) {
        QMessageBox::critical(parent, tr("Error"), tr("No input directory selected."));  // TODO Error as const
        return false;
    }

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
        QMessageBox::critical(parent, tr("Error"), tr("The following input folders don't exist (or are files). Please fix the list under \"Settings\" window:") + "\n" + dirlist);
        return false;
    }

    return true;
}

void GuiTimestamperProcessor::readSettings() {
    QSettings ini(iniPath, QSettings::IniFormat);
    qDebug() << "status " << ini.status() << ini.fileName();

    timeServerUrl = ini.value(Config::INI_PARAM_TIME_SERVER_URL, config.getTimeServerURL()).toString();
    outExt = config.getOutExtension();
    previewFiles = ini.value(INI_PARAM_PREVIEW_FILES, false).toBool();

    exclDirs.unite(config.getExclDirsXXXXXXXX());
    centralExclDirsDisabledByUser.unite(config.getExclDirExclusions());

    showIntro = settings.value(REG_PARAM_SHOW_INTRO, QVariant(true)).toBool();
}

QString asPathList(QSet<QString> set) {
    QList<QString> list(set.toList());
    QString res;
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (res.length() > 0) res += PATH_LIST_SEPARATOR;
        res += *it;
    }
    return res;
}

void GuiTimestamperProcessor::saveSettings() {
    QSettings ini(iniPath, QSettings::IniFormat);

    if (timeServerUrl != config.getDefaultTimeServerURL() && !timeServerUrl.isEmpty()) {
        ini.setValue(Config::INI_PARAM_TIME_SERVER_URL, timeServerUrl);
    } else {
        ini.remove(Config::INI_PARAM_TIME_SERVER_URL);
    }
    // don't store extension at the moment
    ini.setValue(INI_PARAM_PREVIEW_FILES, previewFiles);

    ini.setValue(Config::INI_PARAM_EXCL_DIRS, asPathList(exclDirs));

    QSet<QString> exclDirExcls(centralExclDirs);
    exclDirExcls.subtract(exclDirs);
    if (exclDirExcls.isEmpty()) ini.remove(Config::INI_PARAM_EXCL_DIRS_EXCEPTIONS);
    else ini.setValue(Config::INI_PARAM_EXCL_DIRS_EXCEPTIONS, asPathList(exclDirExcls));

    // save
    ini.sync();
    saveShowIntro(showIntro);
}

QSet<QString>& GuiTimestamperProcessor::_getInclDirs() {
    if (inclDirs.isNull()) inclDirs.reset(new QSet<QString>(config.getDefaultInclDirs()));
    return *inclDirs;
}

QSet<QString> const& GuiTimestamperProcessor::_getInclDirs() const {
    return const_cast<GuiTimestamperProcessor*>(this)->_getInclDirs();
}

QList<QString> GuiTimestamperProcessor::getInclDirList() const {
#ifdef Q_OS_OSX
    QSet<QString> includedDirs(_getInclDirs());

    if (!revoked.isNull()) {
        includedDirs.subtract(*revoked);
    }
    if (!granted.isNull()) {
        includedDirs.unite(*granted);
    }

    // Keep folders sorted, parent folders before children
    auto included = includedDirs.toList();
    std::sort(included.begin(), included.end());
    return included;
#else
    return _getInclDirs().toList();
#endif
}

#ifdef Q_OS_OSX
void GuiTimestamperProcessor::resetGrants(QScopedPointer<QSet<QString>> &removed,
                                          QScopedPointer<QSet<QString>> &added) {
    revoked.reset(removed.take());
    granted.reset(added.take());
}
#endif
}
