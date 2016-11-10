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
    DiskCrawler(ProcessingMonitorCallback& mon, QString const& dir, QStringList const& exclDirs);
    QStringList crawl();
private:
    ProcessingMonitorCallback& monitor;
    QString in_dir;
    QStringList excl_dirs;
};

}

#endif /* DISK_CRAWLER_H_ */
