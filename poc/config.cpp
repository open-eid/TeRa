/*
 * config.cpp
 *
 *  Created on: Nov 18, 2016
 */

#include "config.h"

#include <iostream>

#include <QDebug>
#include <QStorageInfo>

#include "utils.h"

namespace ria_tera {

QString const Config::INI_FILE_NAME("terapoc.ini");
QString const Config::INI_GROUP("tera");
QString const Config::INI_GROUP_ = INI_GROUP + "/";

QString const Config::EXTENSION_IN = "ddoc";
QString const Config::EXTENSION_BDOC = "bdoc";
QString const Config::EXTENSION_ASICS = "asics";
QString const Config::DEFAULT_OUT_EXTENSION = Config::EXTENSION_ASICS;

Config::Config() : settings(INI_FILE_NAME, QSettings::IniFormat)
{
    settingsKeys = settings.allKeys();
}

Config::~Config() {
    ;
}

QSet<QString> Config::readExclDirs() {
    QSet<QString> excl_dirs_set;

    QString const key = INI_GROUP_ + "excl_dir";
    if (settings.contains(key)) {
        QString val = settings.value(key).toString();
        append_excl_dirs(val, excl_dirs_set);
    }

    QString const key_ = key + ".";
    for (int i = 0; i < settingsKeys.length(); ++i) {
        QString k = settingsKeys.at(i);
        if (k.startsWith(key_)) {
            QString val = settings.value(k).toString();
            append_excl_dirs(val, excl_dirs_set);
        }
    }

    return excl_dirs_set;
}

QString Config::readTimeServerURL() {
    return settings.value(ria_tera::Config::INI_GROUP_ + "time_server.url").toString().trimmed();
}

QString Config::readOutExtension() {
    return settings.value("output_format", DEFAULT_OUT_EXTENSION).toString();
}


void Config::append_excl_dirs(QString const& val, QSet<QString>& excl_dirs_set) { // TODO should be in disk_crawler or smth
#ifdef Q_OS_WIN32
    QString const PATH_LIST_SEPARATOR = ";";
#else
    QString const PATH_LIST_SEPARATOR = ":";
#endif
    QStringList paths = val.split(PATH_LIST_SEPARATOR, QString::SkipEmptyParts);
    for (int i = 0; i < paths.size(); ++i) {
        QString path = ria_tera::fix_path(paths.at(i));
        if (!excl_dirs_set.contains(path)) {
            excl_dirs_set.insert(path);
        }
    }
}

QSet<QString> Config::getDefaultInclDirs() {
    QSet<QString> res;
#ifdef Q_OS_WIN32
    const QByteArray LOCAL_PREFIX("\\\\?\\");
    // res.insert(ria_tera::fix_path("~")); // TODO
    QList<QStorageInfo> vols = QStorageInfo::mountedVolumes();
    for (int i = 0; i < vols.length(); ++i) {
        QStorageInfo vol = vols.at(i);

        // "\\\\?\\" prefix is used for local storage only
        if (vol.isReady() && !vol.isReadOnly() && vol.device().startsWith(LOCAL_PREFIX)) {
            res.insert(vol.rootPath());
        }
    }
#else
    res.insert(ria_tera::fix_path("~"));
#endif
    return res;
}

}
