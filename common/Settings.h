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

#include <QtCore/QSettings>

#include <QtCore/QCoreApplication>
#include <QtCore/QLocale>

class Settings : public QSettings
{
	Q_OBJECT

public:
#ifdef Q_OS_MAC
	Settings( const QString & = "" ): QSettings() {}
	Settings(QSettings::Scope scope): QSettings(scope, qApp->organizationDomain(), qApp->applicationName()) {}
#else
	Settings( QObject *parent = 0 )
	: QSettings( "Estonian ID Card", QString(), parent ) {}
	Settings( const QString &application, QObject *parent = 0 )
	: QSettings( "Estonian ID Card", application, parent ) {}
	Settings(QSettings::Scope scope): QSettings(scope, "Estonian ID Card", qApp->applicationName()) {}
#endif

	void setValueEx( const QString &key, const QVariant &value, const QVariant &def )
	{
		if( value == def )
			remove( key );
		else
			setValue( key, value );
	}

	static QString language()
	{
		QString deflang;
		switch( QLocale().language() )
		{
		case QLocale::Russian: deflang = "ru"; break;
		case QLocale::Estonian: deflang = "et"; break;
		case QLocale::English:
		default: deflang = "en"; break;
		}
		return Settings().value( "Main/Language", deflang ).toString();
	}
};
