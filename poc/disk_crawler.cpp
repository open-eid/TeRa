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

DiskCrawler::DiskCrawler(ProcessingMonitorCallback& mon, QString const& dir, QStringList const& exclDirs) :
    monitor(mon), in_dir(dir), excl_dirs(exclDirs) {
}

QStringList DiskCrawler::crawl() {
    QStringList res;

    QStringList nameFilter;
    nameFilter << "*.ddoc";

    DirIterator it(monitor, in_dir, excl_dirs);
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
