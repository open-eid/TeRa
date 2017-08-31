/*
 * TeRa
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

// TODO code-review
#ifndef UTILS_H_
#define UTILS_H_

#include <QObject>
#include <QSet>
#include <QStack>

#define QSTR_TO_CCHAR(str) ((str).toUtf8().constData())

namespace ria_tera {

extern QString const OS_SHORT;
extern QString const PATH_LIST_SEPARATOR;

bool isSubfolder(QString const& path, QSet<QString> const& refDirs);
QString fix_path(QString const& path);
QString hrPath(QString const& path);
QString hrSize(qint64 bytes);

class DiscCrawlMonitorCallback {
public:
    virtual bool processingPath(QString const& path, double progress_percent) = 0;
    virtual bool excludingPath(QString const& path) = 0;
    virtual bool foundFile(QString const& path) = 0;
};

class StampingMonitorCallback {
public:
    virtual bool processingFile(QString const& pathIn, QString const& pathOut, int nr, int totalCnt) = 0;
    virtual bool processingFileDone(QString const& pathIn, QString const& pathOut, int nr, int totalCnt, bool success, QString const& errString) = 0;
};

class ProcessingMonitorCallback : public DiscCrawlMonitorCallback, public StampingMonitorCallback {
};

class DirIterator {
public:
    struct InDir {
        InDir(bool r, QString p) : recursive(r), path(p) {};
        bool recursive;
        QString path;
    };
};

}

#endif /* UTILS_H_ */
