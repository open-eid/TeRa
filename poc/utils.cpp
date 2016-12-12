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
        if (!monitor.processingPath(nextPath, 0)) {
            // cancelling
            nextPath = QString();
            stack.clear();
            return;
        }
        e.dirs.pop_front();

        if (!e.recursive) break;

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
        stack.push(StackEntry(dirCanonicalPath, true, subDirsToVisit));
        break;
    }
}

DirIterator::DirIterator(ProcessingMonitorCallback& mon, QList<InDir> const& inDirs, QStringList const& excl) : monitor(mon) {
    for (auto it = excl.begin(); it != excl.end(); ++it) {
        QFileInfo fi(fix_path(*it));
        exclPaths.insert(fi.canonicalFilePath());
    }
    processInDirs(inDirs);
    findNext();
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

QSet<QString> filterOutSubfolders(QSet<QString> const& dirs, QSet<QString> const& refDirs) {
    QSet<QString> res;
    QList<QString> list = dirs.toList();
    for (int i = 0; i < list.size(); ++i) {
        QString path = list.at(i);
        if (!isSubfolder(path, refDirs)) {
            res.insert(path);
        }
    }
    return res;
}


void DirIterator::processInDirs(QList<InDir> const& inDirs) {
    // first filtering
    QSet<QString> recDirs;
    QSet<QString> nonrecDirs;
    for (int i = 0; i < inDirs.size(); ++i) {
        QFileInfo dir(fix_path(inDirs.at(i).path));
        if (!dir.exists() || !dir.isDir()) continue;

        QString can = dir.canonicalFilePath();
        if (exclPaths.contains(can)) continue; // TODO

        if (inDirs.at(i).recursive) {
            recDirs.insert(can);
        } else {
            nonrecDirs.insert(can);
        }
    }

    recDirs = filterOutSubfolders(recDirs, recDirs);
    nonrecDirs = filterOutSubfolders(nonrecDirs, recDirs);
    nonrecDirs.subtract(recDirs);

    addInputDirs(false, nonrecDirs);
    addInputDirs(true, recDirs);
}

void DirIterator::addInputDirs(bool recursive, QSet<QString> const& dirs) {
    QList<QString> list = dirs.toList();
    qSort(list);
    for (int i = 0; i < list.size(); ++i) {
        stack.push(StackEntry("", recursive, QStringList() << list.at(i)));
    }
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
