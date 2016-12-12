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
    static QString const INI_FILE_NAME;
    static QString const INI_GROUP;
    static QString const INI_GROUP_;

    static QString const EXTENSION_IN;
    static QString const EXTENSION_BDOC;
    static QString const EXTENSION_ASICS;
    static QString const DEFAULT_OUT_EXTENSION;

    Config();
    virtual ~Config();

    QSet<QString> readExclDirs();
    QString readTimeServerURL();
    QString readOutExtension();
	
	QSet<QString> getDefaultInclDirs();

    QSettings& getInternalSettings() {return settings;}; // TODO

    static void append_excl_dirs(QString const& val, QSet<QString>& excl_dirs_set);
private:
    QSettings settings;
    QStringList settingsKeys;
};

}

#endif /* CONFIG_H_ */
