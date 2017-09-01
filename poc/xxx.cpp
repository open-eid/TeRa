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

#include <iostream>

#include <QApplication>
#include <QLoggingCategory>
#include <QThreadPool>

#ifdef Q_OS_WIN32
#include <QtCore/QDebug>
#include <QtCore/qt_windows.h>
#endif

#include "../src/version.h"

#include "main_window.h"
#include "utils.h"

int main(int argc, char *argv[]) {
#if QT_VERSION > QT_VERSION_CHECK(5, 6, 0)
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#ifdef Q_OS_WIN32
	SetProcessDPIAware();
	HDC screen = GetDC(0);
	qreal dpix = GetDeviceCaps(screen, LOGPIXELSX);
	qreal dpiy = GetDeviceCaps(screen, LOGPIXELSY);
	qreal scale = dpiy / qreal(96);
	qputenv("QT_SCALE_FACTOR", QByteArray::number(scale));
	ReleaseDC(NULL, screen);
	qDebug() << "Current DPI x: " << dpix << " y: " << dpiy << " setting scale:" << scale;
#else
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#endif
#endif
    qsrand(QTime::currentTime().msec());

    //QGuiApplication::setAttribute(Qt::AA_Use96Dpi, true);
    //QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling, false);
    //QGuiApplication::setAttribute(Qt::AA_DisableHighDpiScaling, false);
    //QApplication::setDesktopSettingsAware(false);
    QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");
    //qputenv("QT_QPA_PLATFORM_PLUGIN_PATH", ".");
    QApplication a(argc, argv);
    a.setOrganizationName("RIA");
    a.setApplicationName("TeRa");
    a.setApplicationVersion(ria_tera::TERA_CLIENT_VERSION);

    ria_tera::TeraMainWin w;
    w.show();

    int res = a.exec();
    QThreadPool::globalInstance()->waitForDone(); // TODO

    return res;
}


