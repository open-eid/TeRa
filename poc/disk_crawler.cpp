/* TODO
 * disk_crawler.cpp
 *
 *  Created on: Nov 2, 2016
 */
// TODO code-review
#include "disk_crawler.h"

#include <iostream>

#include <QDir>
#include <QFileInfoList>
#include <QSet>
#include <QStack>

namespace ria_tera {

DiskCrawler::DiskCrawler(ProcessingMonitorCallback& mon, QString const& ext) :
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

    QStringList nameFilter;
    nameFilter << ("*." + extension);

    DirIterator it(monitor, in_dirs, excl_dirs);
    while (it.hasNext()) {
        QString subDirPath = it.next();

        QFileInfoList files = QDir(subDirPath).entryInfoList(nameFilter, QDir::Files); // TODO filters sym? hidden?

        for (int i = 0; i < files.size(); ++i) {
            QString filePath = files[i].absoluteFilePath();
            monitor.foundFile(filePath);
            res << filePath;
        }
    }

    return res;
}

}
