/*
 * utils.cpp
 *
 *  Created on: Nov 4, 2016
 */
// TODO code-review
#include "utils.h"

#include <iostream>

#include <QCoreApplication>
#include <QDir>

#include "logging.h"

namespace ria_tera {

void ExitProgram::exitOnFinished(bool success, QString errString) {
    if (success) {
        BOOST_LOG_TRIVIAL(info) << "Timestamping finished successfully :)";
        std::cout << "Finished successfully" << std::endl;
        QCoreApplication::exit(0);
    } else {
        BOOST_LOG_TRIVIAL(error) << "Error: " << errString.toUtf8().constData();
        QCoreApplication::exit(1);
    }
}

bool isSubfolder(QString const& path, QSet<QString> const& refDirs) {
    QString p = path;
    while (true) {
        int pos = p.lastIndexOf(QDir::separator());
        if (pos <= 0 || pos == p.size()-1) break; // root folder

        // found parent
        QString parent = p.left(pos);
        if (refDirs.contains(parent)) {
            return true;
        }

        p = parent;
    }
    return false;
}

QString fix_path(QString const& path) {
    if ("~" == path) return QDir::homePath();
    if (path.startsWith("~/")) return QDir::homePath() + path.mid(1);
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
