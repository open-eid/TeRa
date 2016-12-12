/*
 * xxx.cpp
 */

#include <iostream>

#include <QApplication>
#include <QLoggingCategory>
#include <QThreadPool>

#include "main_window.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");
    qApp->addLibraryPath(".");
    QApplication a(argc, argv);

    ria_tera::TeraMainWin w;
    w.show();

    int res = a.exec();
    QThreadPool::globalInstance()->waitForDone(); // TODO

    return res;
}


