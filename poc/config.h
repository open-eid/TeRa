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

#ifndef CONFIG_H_
#define CONFIG_H_

#include <QSettings>
#include <QSet>
#include <QSslCertificate>

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
    static QString const INI_PARAM_TRUSTED_CERT;

    static QString const EXTENSION_BDOC;
    static QString const EXTENSION_DDOC;
    static QString const EXTENSION_ASICS;
    static QString const DEFAULT_OUT_EXTENSION;
    static QStringList const IN_EXTENSIONS;

    Config();
    virtual ~Config();

    void appendIniFile(QString const& path);

    QString getDefaultTimeServerURL();

    QString getTimeServerURL();
    QString getOutExtension(); // TODO no read
    QSet<QString> getExclDirsXXXXXXXX();
    QSet<QString> getExclDirExclusions();

    QSet<QString> getDefaultInclDirs() const;

    QList<QSslCertificate> getTrustedHttpsCerts();

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
