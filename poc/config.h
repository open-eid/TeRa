/*
 * config.h
 *
 *  Created on: Nov 18, 2016
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <QSettings>
#include <QSet>

namespace ria_tera {

class Config {
public:
    static QString const INI_FILE_DEFAULTS;

    static QString const INI_FILE_NAME;
    static QString const INI_GROUP;
    static QString const INI_GROUP_;

    static QString const INI_PARAM_TIME_SERVER_URL;
    static QString const INI_PARAM_OUTPUT_FORMAT;
    static QString const INI_PARAM_EXCL_DIRS;
    static QString const INI_PARAM_EXCL_DIRS_EXCEPTIONS;

    static QString const EXTENSION_IN;
    static QString const EXTENSION_BDOC;
    static QString const EXTENSION_ASICS;
    static QString const DEFAULT_OUT_EXTENSION;

    Config();
    virtual ~Config();

    void appendIniFile(QString const& path);

    QString getDefaultTimeServerURL();

    QString getTimeServerURL();
    QString getOutExtension(); // TODO no read
    QSet<QString> getExclDirsXXXXXXXX();
    QSet<QString> getExclDirExclusions();

    QSet<QString> getDefaultInclDirs() const;

    static void append_excl_dirs(QString const& val, QSet<QString>& excl_dirs_set); // TODO no need to be public
private:
    static QSet<QString> readExclDirs(QString const& key, QSettings const& settings);

    QString timeServerURLDefault;

    QString outExtension;
    QString timeServerURL;
    QSet<QString> exclDirs;
    QSet<QString> exclDirExclusions;
};

}

#endif /* CONFIG_H_ */
