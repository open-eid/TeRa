/*
 * xxx.cpp
 */

#include <iostream>

#include <QApplication>
#include <QLoggingCategory>

#include "main_window.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");
    QApplication a(argc, argv);

    ria_tera::TeraMainWin w;
    w.show();

    return a.exec();
}


