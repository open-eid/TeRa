/*
 * disk_crawler.h
 *
 *  Created on: Nov 2, 2016
 */

#ifndef DISK_CRAWLER_H_
#define DISK_CRAWLER_H_

#include <QStringList>

namespace ria_tera {

class DiskCrawler
{
public:
    DiskCrawler(QString const& dir, QStringList const& exclDirs);
    QStringList crawl();
private:
    QString in_dir;
    QStringList excl_dirs;
};

}

#endif /* DISK_CRAWLER_H_ */
