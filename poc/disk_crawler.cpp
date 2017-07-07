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

#include "config.h"
#include "../src/common/Bdoc10Handler.h"

namespace {

bool inExclDirs(QString const& filePath, QStringList const& excldir) {
    for (int i = 0; i < excldir.size(); ++i) {
        if (filePath.startsWith(excldir.at(i))) return true;
    }
    return false;
}

}

namespace ria_tera {

DiskCrawler::DiskCrawler(DiscCrawlMonitorCallback& mon, QStringList const& ext) :
    monitor(mon), extensions(ext) {
}

void DiskCrawler::addExcludeDirs(QStringList const& excl) {
    excl_dirs << excl;
}

bool DiskCrawler::addInputDir(QString const& dir, bool rec) {
    in_dirs.append(DirIterator::InDir(rec, dir));
    return true; // TODO check if dir is actually excluded
}

QStringList DiskCrawler::crawl() {
    const QString EXTENSION_BDOC_WITH_DOT("." + ria_tera::Config::EXTENSION_BDOC);

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
    }

    QStringList nameFilter;
    foreach (QString ext, extensions) {
        nameFilter << ("*." + ext);
    }

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

            //in case of BDOC only BDOC1.0 need be processed
            if (filePath.endsWith(EXTENSION_BDOC_WITH_DOT)) {
                if (!Bdoc10Handler::isBdoc10Container(filePath)) {
                    continue;
                }
            }
            monitor.foundFile(filePath);  // TODO cancel returns false
            res << filePath;
        }
    }

    return res;
}

}
