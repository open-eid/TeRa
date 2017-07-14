/*
 * disk_crawler.h
 *
 *  Created on: Nov 2, 2016
 */
// TODO ll
#ifndef DISK_CRAWLER_H_
#define DISK_CRAWLER_H_

#include <QStringList>

#include "utils.h"

namespace ria_tera {

class DiskCrawler
{
public:
    DiskCrawler(DiscCrawlMonitorCallback& mon, QStringList const& ext);
    void addExcludeDirs(QStringList const& excl);
    bool addInputDir(QString const& dir, bool recursive);
    QStringList crawl();
private:
    /// file extension to search
    QStringList extensions;
    DiscCrawlMonitorCallback& monitor;
    QList<DirIterator::InDir> in_dirs;
    QStringList excl_dirs;
};

}

#endif /* DISK_CRAWLER_H_ */
