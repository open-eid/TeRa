/*
 * utils.cpp
 *
 *  Created on: Nov 4, 2016
 */
// TODO code-review
#include "utils.h"

#include <iostream>

#ifdef Q_OS_OSX
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
    #include <assert.h>

    #include "utils_mac.h"
#endif

#include <QCoreApplication>
#include <QDebug>
#include <QDir>

#include "logging.h"

#ifdef Q_OS_OSX
namespace {

QString RealMacOSHomeDirectory() {
    struct passwd *pw = getpwuid(getuid());
    if (!pw) return nullptr;
    return pw->pw_dir;
}

};
#endif

namespace ria_tera {

#ifdef Q_OS_WIN32
QString const PATH_LIST_SEPARATOR(";");
QString const OS_SHORT("WIN");
#else
QString const PATH_LIST_SEPARATOR = ":";
  #ifdef Q_OS_OSX
  QString const OS_SHORT("OSX");
  #else
  QString const OS_SHORT("UBUNTU");
  #endif
#endif

bool isSubfolder(QString const& path, QSet<QString> const& refDirs) {
    QString p = path;
    while (true) {
        int pos = p.lastIndexOf(QDir::separator());
        if (pos <= 0 || pos == p.size()-1) break; // root folder

        // found parent
        QString parent = p.left(pos);
        if (refDirs.contains(parent) ||
            refDirs.contains(parent + QDir::separator())) {
            return true;
        }

        p = parent;
    }
    return false;
}

QString fix_path(QString const& path) {
#ifdef Q_OS_OSX
    QString const home = RealMacOSHomeDirectory();
#else
    QString const home = QDir::homePath();
#endif
    if ("~" == path) return home;
    if (path.startsWith("~/")) return home + path.mid(1);
    return path;
}

QString hrPath(QString const& path) {
    return QString(path).replace("/", "\\");
}

QString hrSize(qint64 bytes) {
    static const int K = 1024;
    if (bytes < K) return QString::number(bytes) + " bytes";

    double size = bytes;
    size = size / K;
    if (size < K) return QString::number(size, 'f', 1) + " KB";

    size = size / K;
    if (size < K) return QString::number(size, 'f', 1) + " MB";

    size = size / K;
    return QString::number(size, 'f', 1) + " GB";
}

}
