/* TODO
 * disk_crawler.cpp
 *
 *  Created on: Nov 2, 2016
 */
// TODO code-review
#include "disk_crawler.h"

#include <iostream>

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfoList>
#include <QSet>
#include <QStack>

namespace {

bool inExclDirs(QString const& filePath, QStringList const& excldir) {
    for (int i = 0; i < excldir.size(); ++i) {
        if (filePath.startsWith(excldir.at(i))) return true;
    }
    return false;
}

}

namespace ria_tera {

DiskCrawler::DiskCrawler(DiscCrawlMonitorCallback& mon, QString const& ext) :
    monitor(mon), extension(ext) {
}

void DiskCrawler::addExcludeDirs(QStringList const& excl) {
    excl_dirs << excl;
}

bool DiskCrawler::addInputDir(QString const& dir, bool rec) {
    in_dirs.append(DirIterator::InDir(rec, dir));
    return true; // TODO check if dir is actually excluded
}

QStringList DiskCrawler::crawl() {
    QStringList res;

    QStringList excldir;
    const QString separator("/");
    for (int i = 0; i < excl_dirs.size(); ++i) {
        QString dir_name = fix_path(excl_dirs.at(i));
        QFileInfo fi(dir_name);

        if (!fi.isDir()) continue;
        QString name = fi.absoluteFilePath();
        if (!name.endsWith(separator)) name += separator;

        excldir.append(name);
qDebug() << "Excl-dir: " << name;
    }

    QStringList nameFilter;
    nameFilter << ("*." + extension);

    for (int i = 0; i < in_dirs.length(); ++i) {
        DirIterator::InDir in_dir = in_dirs.at(i);
        QDirIterator::IteratorFlags flags;

        if (in_dir.recursive) {
            flags |= QDirIterator::Subdirectories;
        }

        if (!monitor.processingPath(in_dir.path, (double)i / in_dirs.length())) return res; // TODO cancel

        QDirIterator it(in_dir.path, nameFilter, QDir::Files, flags);
        while (it.hasNext()) {
            it.next();
            QString filePath = it.fileInfo().absoluteFilePath();

            if (inExclDirs(filePath, excldir)) {
                monitor.excludingPath(filePath);
                continue;
            }

            monitor.foundFile(filePath);
            res << filePath;
        }
    }

    return res;
}

}
