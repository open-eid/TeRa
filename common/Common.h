/*
 * QEstEidCommon
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

#pragma once

#include <QtCore/QtGlobal>

#ifdef Q_OS_MAC
#include <QtWidgets/QApplication>
typedef QApplication BaseApplication;
#else
#include "qtsingleapplication/src/QtSingleApplication"
typedef QtSingleApplication BaseApplication;
#endif

#include <QtCore/QEvent>


#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<Common*>(QCoreApplication::instance()))

class QLabel;
class QUrl;

class Common: public BaseApplication
{
	Q_OBJECT
public:
#ifndef COMMON_STATIC
	Common( int &argc, char **argv, const QString &app, const QString &icon );
#endif

	bool isCrashReport();
	virtual void diagnostics(QTextStream &s);

	static QString applicationOs();
	static QUrl helpUrl();
	static QStringList packages( const QStringList &names, bool withName = true );
	static void setAccessibleName( QLabel *widget );

private:
	static void msgHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg);
};
