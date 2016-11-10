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

// fills next path
void DirIterator::findNext() {
    nextPath = QString();

    while (!stack.isEmpty()) {
        StackEntry& e = stack.top();
        if (e.dirs.isEmpty()) { // go up
            pathsInStack.remove(e.parentCanonical);
            stack.pop();
            continue;
        }
        nextPath = e.dirs.first();
        monitor.processingPath(nextPath);
        e.dirs.pop_front();
        // process path = list sub-dirs
        QDir dir(nextPath);
        QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        QStringList subDirsToVisit;
        for (auto it = subDirs.begin(); it != subDirs.end(); ++it) {
            QFileInfo sd(dir, *it);
            QString can = sd.canonicalFilePath();
            if (pathsInStack.contains(can)) continue;
            if (exclPaths.contains(can)) {
                monitor.excludingPath(can);
                continue;
            }
            subDirsToVisit.push_back(sd.filePath());
        }

        QString dirCanonicalPath = dir.canonicalPath();
        pathsInStack.insert(dirCanonicalPath);
        stack.push(StackEntry(dirCanonicalPath, subDirsToVisit));
        break;
    }
}

DirIterator::DirIterator(ProcessingMonitorCallback& mon, QString const& dir, QStringList const& excl) : monitor(mon) {
    stack.push(StackEntry("", QStringList() << dir)); // TODO check out if exists, check out if in excl list
    for (auto it = excl.begin(); it != excl.end(); ++it) {
        QFileInfo fi(*it);
        exclPaths.insert(fi.canonicalFilePath());
    }
    findNext();
}

bool DirIterator::hasNext() {
    return !nextPath.isEmpty();
}

QString DirIterator::next() {
    QString res = nextPath;
    findNext();
    return res;
}

QString fix_path(QString const& path) {
    if ("~" == path) return QDir::homePath();
    if (path.startsWith("~/")) return QDir::homePath() + path.mid(1);
    return path;
}

}
