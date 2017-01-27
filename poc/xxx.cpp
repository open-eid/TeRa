/*
 * xxx.cpp
 */

#include <iostream>

#include <QApplication>
#include <QLoggingCategory>
#include <QThreadPool>

#include "../src/version.h"

#include "main_window.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    //QGuiApplication::setAttribute(Qt::AA_Use96Dpi, true);
    //QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling, false);
    //QGuiApplication::setAttribute(Qt::AA_DisableHighDpiScaling, false);
    //QApplication::setDesktopSettingsAware(false);
    QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");
    //qputenv("QT_QPA_PLATFORM_PLUGIN_PATH", ".");
    QApplication a(argc, argv);
    a.setOrganizationName("RIA");
    a.setApplicationName("tera");
    a.setApplicationVersion(ria_tera::TERA_CLIENT_VERSION);

    ria_tera::TeraMainWin w;
    w.show();

    int res = a.exec();
    QThreadPool::globalInstance()->waitForDone(); // TODO

    return res;
}


