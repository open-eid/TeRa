/*
 * config.cpp
 *
 *  Created on: Nov 18, 2016
 */

#include "config.h"

#include <iostream>

#include <QDebug>
#include <QDirIterator> // TODO

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include "src/libdigidoc/Configuration.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
#else
    #include <QStorageInfo>
#endif

#include "utils.h"

namespace ria_tera {

QString const Config::INI_FILE_DEFAULTS(":/tera.ini");

QString const Config::INI_GROUP("tera");
QString const Config::INI_GROUP_ = INI_GROUP + "/";

QString const Config::INI_PARAM_TIME_SERVER_URL = Config::INI_GROUP_ + "time_server.url";
QString const Config::INI_PARAM_OUTPUT_FORMAT = Config::INI_GROUP_ + "output_format";
QString const Config::INI_PARAM_EXCL_DIRS = Config::INI_GROUP_ + "excl_dir";
QString const Config::INI_PARAM_EXCL_DIRS_EXCEPTIONS = Config::INI_GROUP_ + "central_excl_dirs_removed_by_user.no_need_to_changed_manully";
QString const Config::INI_PARAM_TRUSTED_CERT = Config::INI_GROUP_ + "time_server.trusted_cert";

QString const Config::EXTENSION_DDOC = "ddoc";
QString const Config::EXTENSION_BDOC = "bdoc";
QString const Config::EXTENSION_ASICS = "asics";
QString const Config::DEFAULT_OUT_EXTENSION = Config::EXTENSION_ASICS; // TODO move away from here? or private?
QStringList const Config::IN_EXTENSIONS = {EXTENSION_BDOC, EXTENSION_DDOC};

Config::Config()
{
    outExtension = DEFAULT_OUT_EXTENSION;
    appendIniFile(INI_FILE_DEFAULTS);
    timeServerURLDefault = timeServerURL;
}

Config::~Config() {
    ;
}

void Config::appendIniFile(QString const& ini_path) {
    QSettings settings(ini_path, QSettings::IniFormat);
    timeServerURL = settings.value(INI_PARAM_TIME_SERVER_URL, timeServerURL).toString().trimmed();
    outExtension  = settings.value(INI_PARAM_OUTPUT_FORMAT,   outExtension).toString().trimmed();
    exclDirs.unite(readExclDirs(INI_PARAM_EXCL_DIRS, settings));
    exclDirExclusions.unite(readExclDirs(INI_PARAM_EXCL_DIRS_EXCEPTIONS, settings));
}

QSet<QString> Config::readExclDirs(QString const& key, QSettings const& settings) {
    QStringList settingsKeys = settings.allKeys();
    QSet<QString> excl_dirs_set;

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

QString Config::getDefaultTimeServerURL() {
    return timeServerURLDefault;
}

QString Config::getTimeServerURL() {
    return timeServerURL;
}

QString Config::getOutExtension() {
    return outExtension;
}

QSet<QString> Config::getExclDirsXXXXXXXX() {
    return exclDirs;
}

QSet<QString> Config::getExclDirExclusions() {
    return exclDirExclusions;
}

static QSslCertificate derFromBase64(QByteArray const& der) {
    return QSslCertificate(QByteArray::fromBase64(der), QSsl::Der);
}

QList<QSslCertificate> Config::getTrustedHttpsCerts() {
    QList<QSslCertificate> trusted;

    for (const QJsonValue &cert : Configuration::instance().object().value("CERT-BUNDLE").toArray()) {
        trusted << derFromBase64(cert.toString().toLatin1());
    }

    QSettings settings(":/tera.ini", QSettings::IniFormat);
    QStringList sl  = settings.allKeys();
    QString der = settings.value(INI_PARAM_TRUSTED_CERT).toString();
    if (!der.isNull()) {
        trusted << derFromBase64(der.toLatin1());
    }

    return trusted;
}

void Config::append_excl_dirs(QString const& val, QSet<QString>& excl_dirs_set) { // TODO should be in disk_crawler or smth
    QStringList paths = val.split(PATH_LIST_SEPARATOR, QString::SkipEmptyParts);
    for (int i = 0; i < paths.size(); ++i) {
        QString path = ria_tera::fix_path(paths.at(i));
        if (!excl_dirs_set.contains(path)) {
            excl_dirs_set.insert(path);
        }
    }
}

QSet<QString> Config::getDefaultInclDirs() const {
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
