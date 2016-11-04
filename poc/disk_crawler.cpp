/*
 * disk_crawler.cpp
 *
 *  Created on: Nov 2, 2016
 */

#include "disk_crawler.h"

#include <iostream>

#include <QDir>
#include <QDirIterator>
#include <QFileInfoList>
#include <QSet>
#include <QStack>

namespace ria_tera {

DiskCrawler::DiskCrawler(QString const& dir, QStringList const& exclDirs) :
        in_dir(dir), excl_dirs(exclDirs) {
}

class DirIterator {
private:
    class StackEntry {
    public:
        StackEntry(){};
        StackEntry(QString const& p, QStringList const& d) : parentCanonical(p), dirs(d) {}
        QString parentCanonical;
        QStringList dirs;
    };
    QString nextPath;
    QSet<QString> pathsInStack;
    QStack<StackEntry> stack; // TODO copying
    QSet<QString> exclPaths;

    // fills next path
    void findNext() {
        nextPath = QString();

        while (!stack.isEmpty()) {
            StackEntry& e = stack.top();
            if (e.dirs.isEmpty()) { // go up
                pathsInStack.remove(e.parentCanonical);
                stack.pop();
                continue;
            }
            nextPath = e.dirs.first();
            e.dirs.pop_front();
            // process path = list sub-dirs
            QDir dir(nextPath);
            QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            QStringList subDirsToVisit;
//std::cout << "   .   " << nextPath.toUtf8().constData() << " " << subDirs.size() << std::endl;
            for (auto it = subDirs.begin(); it != subDirs.end(); ++it) {
                QFileInfo sd(dir, *it);
                QString can = sd.canonicalFilePath();
                if (pathsInStack.contains(can)) continue;
                if (exclPaths.contains(can)) continue;
                subDirsToVisit.push_back(sd.filePath());
            }

            QString dirCanonicalPath = dir.canonicalPath();
            pathsInStack.insert(dirCanonicalPath);
            stack.push(StackEntry(dirCanonicalPath, subDirsToVisit));
            break;
        }
    }
public:
    DirIterator(QString const& dir, QStringList const& excl) {
        stack.push(StackEntry("", QStringList() << dir)); // TODO check out if exists, check out if in excl list
        for (auto it = excl.begin(); it != excl.end(); ++it) {
            QFileInfo fi(*it);
std::cout << "kl;jadfiopjdfoadjofj   " << fi.canonicalFilePath().toUtf8().constData() << std::endl;
            exclPaths.insert(fi.canonicalFilePath());
        }
        findNext();
    }
    bool hasNext() {return !nextPath.isEmpty();};
    QString next() {
        QString res = nextPath;
        findNext();
        return res;
    }
};

QStringList DiskCrawler::crawl() {
    QStringList res;

    QStringList nameFilter;
    nameFilter << "*.bdoc";

    //QDirIterator it(in_dir, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    DirIterator it(in_dir, excl_dirs);
    //QFileInfoList subDirs = dir.entryInfoList(QDir::Dirs); // TODO filters sym? hidden? drives? qdiriterator?
    while (it.hasNext()) {
        QString subDirPath = it.next();
std::cout << ".oOo. " << subDirPath.toUtf8().constData() << std::endl;

        QFileInfoList files = QDir(subDirPath).entryInfoList(nameFilter, QDir::Files); // TODO filters sym? hidden?

        for (int i = 0; i < files.size(); ++i) {
            res << files[i].absoluteFilePath(); // TODO
        }
    }

    return res;
}

}
