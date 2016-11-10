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

#define QSTR_TO_CCHAR(str) (str.toUtf8().constData())

namespace ria_tera {

QString fix_path(QString const& path);

class ExitProgram : public QObject {
    Q_OBJECT
public slots:
    void exitOnFinished(bool success, QString errString);
};

class ProcessingMonitorCallback {
public:
    void virtual processingPath(QString const& path) {};
    void virtual excludingPath(QString const& path) {};
    void virtual foundFile(QString const& path) {};
    void virtual processingFile(QString const& pathIn, QString const& pathOut, int nr, int totalCnt) {};
};

class DirIterator {
public:
    DirIterator(ProcessingMonitorCallback& mon, QString const& dir, QStringList const& excl);
    bool hasNext();
    QString next();
private:
    class StackEntry {
    public:
        StackEntry(){};
        StackEntry(QString const& p, QStringList const& d) : parentCanonical(p), dirs(d) {}
        QString parentCanonical;
        QStringList dirs;
    };
    ProcessingMonitorCallback& monitor;
    QString nextPath;
    QSet<QString> pathsInStack;
    QStack<StackEntry> stack; // TODO copying
    QSet<QString> exclPaths;
    // fills next path
    void findNext();
};

}

#endif /* UTILS_H_ */
