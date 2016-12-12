/*
 * utils.h
 *
 *  Created on: Nov 4, 2016
 */
// TODO code-review
#ifndef UTILS_H_
#define UTILS_H_

#include <QObject>
#include <QSet>
#include <QStack>

#define QSTR_TO_CCHAR(str) ((str).toUtf8().constData())

namespace ria_tera {

bool isSubfolder(QString const& path, QSet<QString> const& refDirs);
QString fix_path(QString const& path);

class ExitProgram : public QObject {
    Q_OBJECT
public slots:
    void exitOnFinished(bool success, QString errString);
};

class DiscCrawlMonitorCallback {
public:
    virtual bool processingPath(QString const& path, double progress_percent) { return true; };
    virtual bool excludingPath(QString const& path) { return true; };
    virtual bool foundFile(QString const& path) { return true; };
};

class ProcessingMonitorCallback : public DiscCrawlMonitorCallback {
public:
    virtual bool processingFile(QString const& pathIn, QString const& pathOut, int nr, int totalCnt) {return true;};
};

class DirIterator {
public:
    struct InDir {
        InDir(bool r, QString p) : recursive(r), path(p) {};
        bool recursive;
        QString path;
    };
    DirIterator(ProcessingMonitorCallback& mon, QList<InDir> const& inDirs, QStringList const& excl);
    bool hasNext();
    QString next();
private:
    class StackEntry {
    public:
        StackEntry(){};
        StackEntry(QString const& p, bool r, QStringList const& d) : parentCanonical(p), recursive(r), dirs(d) {}
        QString parentCanonical;
        bool recursive;
        QStringList dirs;
    };
    ProcessingMonitorCallback& monitor;
    QString nextPath;
    QSet<QString> pathsInStack;
    QStack<StackEntry> stack; // TODO copying
    QSet<QString> exclPaths;

    void processInDirs(QList<InDir> const& inDirs);
    void addInputDirs(bool recursive, QSet<QString> const& dirs);
    /// fills next path
    void findNext();
};

}

#endif /* UTILS_H_ */
