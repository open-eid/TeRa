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

#include "Common.h"

#include "SslCertificate.h"
#include "TokenData.h"
#include "Settings.h"
#ifdef BREAKPAD
#include "QBreakPad.h"
#endif

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtGui/QDesktopServices>
#include <QtGui/QPalette>
#include <QtGui/QTextDocument>
#include <QtNetwork/QNetworkProxyFactory>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include <stdlib.h>

#if defined(Q_OS_WIN)
#include <QtCore/QLibrary>
#include <qt_windows.h>
#elif defined(Q_OS_MAC)
#include <QtCore/QXmlStreamReader>
#include <sys/utsname.h>
#include <CoreFoundation/CFBundle.h>
#endif

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
static QString packageName( const QString &name, const QString &ver, bool withName )
{ return withName ? name + " (" + ver + ")" : ver; }
#endif

#ifndef COMMON_STATIC
Common::Common( int &argc, char **argv, const QString &app, const QString &icon )
	: BaseApplication( argc, argv )
{
	setApplicationName( app );
	setApplicationVersion( QString( "%1.%2.%3.%4" )
		.arg( MAJOR_VER ).arg( MINOR_VER ).arg( RELEASE_VER ).arg( BUILD_VER ) );
	setOrganizationDomain( "ria.ee" );
	setOrganizationName( ORG );
	setWindowIcon( QIcon( icon ) );
	if( QFile::exists( QString("%1/%2.log").arg( QDir::tempPath(), app ) ) )
		qInstallMessageHandler(msgHandler);

#ifdef BREAKPAD
	new QBreakPad(this);
#ifdef TESTING
	if( arguments().contains( "-crash" ) )
	{
		QBreakPad *crash;
		delete crash;
	}
#endif
#endif

	Q_INIT_RESOURCE(common_images);
	Q_INIT_RESOURCE(common_tr);
#if defined(Q_OS_WIN)
	setLibraryPaths( QStringList() << applicationDirPath() );
#elif defined(Q_OS_MAC)
	setLibraryPaths( QStringList() << applicationDirPath() + "/../PlugIns" );
#endif
	setStyleSheet(
		"QDialogButtonBox { dialogbuttonbox-buttons-have-icons: 0; }\n" );
	QPalette p = palette();
	p.setBrush( QPalette::Link, QBrush( "#509B00" ) );
	p.setBrush( QPalette::LinkVisited, QBrush( "#509B00" ) );
	setPalette( p );

	qRegisterMetaType<TokenData>("TokenData");

	QNetworkProxyFactory::setUseSystemConfiguration(true);

#if defined(Q_OS_WIN)
	AllowSetForegroundWindow( ASFW_ANY );
#elif defined(Q_OS_MAC)
#ifdef BREAKPAD
	if(arguments().contains("-crashreport", Qt::CaseInsensitive))
		return;
#endif
	if(!QSettings().value("plugins").isNull())
		return;

	QTimer *timer = new QTimer(this);
	timer->setSingleShot(true);
	connect(timer, &QTimer::timeout, this, [=]{
		timer->deleteLater();
		QMessageBox *b = new QMessageBox(QMessageBox::Information, tr("Browser plugins"),
			tr("If you are using e-services for authentication and signing documents in addition to "
				"Mobile-ID an ID-card or only ID-card, you should install the browser integration packages.<br />"
				"<a href='http://installer.id.ee'>http://installer.id.ee</a>"),
			0, activeWindow());
		b->addButton(tr("Remind later"), QMessageBox::RejectRole);
		b->addButton(tr("Ignore forever"), QMessageBox::AcceptRole);
		if(b->exec() == QDialog::Accepted)
			QSettings().setValue("plugins", "ignore");
	});
	timer->start(1000);
#endif
}
#endif

QString Common::applicationOs()
{
#if defined(Q_OS_LINUX)
	QProcess p;
	p.start( "lsb_release", QStringList() << "-s" << "-d" );
	p.waitForFinished();
	return QString::fromLocal8Bit( p.readAll().trimmed() );
#elif defined(Q_OS_MAC)
	struct utsname unameData;
	uname( &unameData );
	QFile f( "/System/Library/CoreServices/SystemVersion.plist" );
	if( f.open( QFile::ReadOnly ) )
	{
		QXmlStreamReader xml( &f );
		while( xml.readNext() != QXmlStreamReader::Invalid )
		{
			if( !xml.isStartElement() || xml.name() != "key" || xml.readElementText() != "ProductVersion" )
				continue;
			xml.readNextStartElement();
			return QString( "Mac OS %1 (%2/%3)" )
				.arg( xml.readElementText() ).arg( QSysInfo::WordSize ).arg( unameData.machine );
		}
	}
#elif defined(Q_OS_WIN)
	OSVERSIONINFOEX osvi = { sizeof( OSVERSIONINFOEX ) };
	if( GetVersionEx( (OSVERSIONINFO *)&osvi ) )
	{
		bool workstation = osvi.wProductType == VER_NT_WORKSTATION;
		SYSTEM_INFO si;
		typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
		if( PGNSI pGNSI = PGNSI( QLibrary( "kernel32" ).resolve( "GetNativeSystemInfo" ) ) )
			pGNSI( &si );
		else
			GetSystemInfo( &si );
		QString os;
		switch( (osvi.dwMajorVersion << 8) + osvi.dwMinorVersion )
		{
		case 0x0500: os = workstation ? "2000 Professional" : "2000 Server"; break;
		case 0x0501: os = osvi.wSuiteMask & VER_SUITE_PERSONAL ? "XP Home" : "XP Professional"; break;
		case 0x0502:
			if( GetSystemMetrics( SM_SERVERR2 ) )
				os = "Server 2003 R2";
			else if( workstation && si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 )
				os = "XP Professional";
			else
				os = "Server 2003";
			break;
		case 0x0600: os = workstation ? "Vista" : "Server 2008"; break;
		case 0x0601: os = workstation ? "7" : "Server 2008 R2"; break;
		case 0x0602: os = workstation ? "8" : "Server 2012"; break;
		case 0x0603: os = workstation ? "8.1" : "Server 2012 R2"; break;
		case 0x0A00: os = workstation ? "10" : "Server 10"; break;
		default: break;
		}
		QString extversion( (const QChar*)osvi.szCSDVersion );
		return QString( "Windows %1 %2(%3 bit)" ).arg( os )
			.arg( extversion.isEmpty() ? "" : extversion + " " )
			.arg( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? "64" : "32" );
	}
	else
	{
		switch( QSysInfo::WindowsVersion )
		{
		case QSysInfo::WV_2000: return "Windows 2000";
		case QSysInfo::WV_XP: return "Windows XP";
		case QSysInfo::WV_2003: return "Windows 2003";
		case QSysInfo::WV_VISTA: return "Windows Vista";
		case QSysInfo::WV_WINDOWS7: return "Windows 7";
		case QSysInfo::WV_WINDOWS8: return "Windows 8";
		case QSysInfo::WV_WINDOWS8_1: return "Windows 8.1";
		case QSysInfo::WV_WINDOWS10: return "Windows 10";
		default: break;
		}
	}
#endif

	return tr("Unknown OS");
}

void Common::diagnostics(QTextStream &)
{}

QUrl Common::helpUrl()
{
	QString lang = Settings::language();
	QUrl u( "http://www.id.ee/index.php?id=10583" );
	if( lang == "en" ) u = "http://www.id.ee/index.php?id=30466";
	if( lang == "ru" ) u = "http://www.id.ee/index.php?id=30515";
	return u;
}

bool Common::isCrashReport()
{
#ifdef BREAKPAD
	if(arguments().contains("-crashreport", Qt::CaseInsensitive))
	{
		QBreakPadDialog d(applicationName());
		d.setProperty("User-Agent", QString( "%1/%2 (%3)")
			.arg(applicationName(), applicationVersion(), applicationOs()).toUtf8());
		d.show();
		return true;
	}
#endif
	return false;
}

void Common::msgHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
	QFile f( QString("%1/%2.log").arg( QDir::tempPath(), applicationName() ) );
	if(!f.open( QFile::Append ))
		return;
	f.write(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ").toUtf8());
	switch(type)
	{
	case QtDebugMsg: f.write("D"); break;
	case QtWarningMsg: f.write("W"); break;
	case QtCriticalMsg: f.write("C"); break;
	case QtFatalMsg: f.write("F"); break;
	default: f.write("I"); break;
	}
	f.write(QString(" %1 ").arg(ctx.category).toUtf8());
	if(ctx.line > 0)
	{
		f.write(QString("%1:%2 \"%3\" ")
			.arg(QFileInfo(ctx.file).fileName())
			.arg(ctx.line)
			.arg(ctx.function).toUtf8());
	}
	f.write(msg.toUtf8());
	f.write("\n");
}

QStringList Common::packages( const QStringList &names, bool withName )
{
	QStringList packages;
#if defined(Q_OS_WIN)
	QString path = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
#if 1
	int count = applicationOs().contains( "64" ) ? 4 : 2;
	for( int i = 0; i < count; ++i )
	{
		HKEY reg = i % 2 == 0 ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
		REGSAM param = KEY_READ|(i >= 2 ? KEY_WOW64_32KEY : KEY_WOW64_64KEY);
		HKEY key;
		long result = RegOpenKeyEx( reg, LPCWSTR(path.utf16()), 0, param, &key );
		if( result != ERROR_SUCCESS )
			continue;

		DWORD numSubgroups = 0, maxSubgroupSize = 0;
		result = RegQueryInfoKey( key, 0, 0, 0, &numSubgroups, &maxSubgroupSize, 0, 0, 0, 0, 0, 0 );
		if( result != ERROR_SUCCESS )
		{
			RegCloseKey( key );
			continue;
		}

		for( DWORD j = 0; j < numSubgroups; ++j )
		{
			DWORD groupSize = maxSubgroupSize + 1;
			QString group( groupSize, 0 );
			result = RegEnumKeyEx( key, j, LPWSTR(group.data()), &groupSize, 0, 0, 0, 0 );
			if( result != ERROR_SUCCESS )
				continue;
			group.resize( groupSize );

			HKEY subkey;
			QString subpath = path + "\\" + group;
			result = RegOpenKeyEx( reg, LPCWSTR(subpath.utf16()), 0, param, &subkey );
			if( result != ERROR_SUCCESS )
				continue;

			DWORD numKeys = 0, maxKeySize = 0, maxValueSize = 0;
			result = RegQueryInfoKey( subkey, 0, 0, 0, 0, 0, 0, &numKeys, &maxKeySize, &maxValueSize, 0, 0 );
			if( result != ERROR_SUCCESS )
			{
				RegCloseKey( subkey );
				continue;
			}

			QString name, version, type, location;
			for( DWORD k = 0; k < numKeys; ++k )
			{
				DWORD dataType = 0;
				DWORD keySize = maxKeySize + 1;
				DWORD dataSize = maxValueSize;
				QString key( keySize, 0 );
				QByteArray data( dataSize, 0 );

				result = RegEnumValue( subkey, k, LPWSTR(key.data()), &keySize, 0,
					&dataType, (unsigned char*)data.data(), &dataSize );
				if( result != ERROR_SUCCESS )
					continue;
				key.resize( keySize );
				data.resize( dataSize );

				QString value;
				switch( dataType )
				{
				case REG_SZ:
					value = QString::fromUtf16( (const ushort*)data.constData() );
					break;
				default: continue;
				}

				if( key == "DisplayName" ) name = value;
				if( key == "DisplayVersion" ) version = value;
				if( key == "ReleaseType" ) type = value;
				if( key == "InstallLocation" ) location = value;
			}
			RegCloseKey( subkey );

			if( name.contains("Chrome") )
			{
				QFile f(location + "\\chrome.exe");
				if( f.open(QFile::ReadOnly) )
				{
					QByteArray data = f.read(1024);
					IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER*)data.constData();
					IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS*)(dos->e_lfanew + data.constData());
					if( nt->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64 )
						name += " (64-bit)";
				}
			}

			if( !type.contains( "Update", Qt::CaseInsensitive ) &&
				name.contains( QRegExp( names.join( "|" ), Qt::CaseInsensitive ) ) )
				packages << packageName( name, version, withName );
		}
		RegCloseKey( key );
	}
	packages.removeDuplicates();
#else // problems on 64bit windows
	static const QStringList roots{"HKEY_LOCAL_MACHINE", "HKEY_CURRENT_USER"},
	for(const QString &group: roots)
	{
		QSettings s( group + "\\" + path, QSettings::NativeFormat );
		for(const QString &key: s.childGroups())
		{
			QString name = s.value( key + "/DisplayName" ).toString();
			QString version = s.value( key + "/DisplayVersion" ).toString();
			QString type = s.value( key + "/ReleaseType" ).toString();
			if( !type.contains( "Update", Qt::CaseInsensitive ) &&
				name.contains( QRegExp( names.join( "|" ), Qt::CaseInsensitive ) ) )
				packages << packageName( name, version, withName );
		}
	}
#endif
#elif defined(Q_OS_MAC)
	Q_UNUSED(withName);
	for (const QString &name: names) {
		CFStringRef id = QString("ee.ria." + name).toCFString();
		CFBundleRef bundle = CFBundleGetBundleWithIdentifier(id);
		CFRelease(id);
		if (!bundle)
			continue;
		CFStringRef ver = (CFStringRef)CFBundleGetValueForInfoDictionaryKey(bundle, CFSTR("CFBundleShortVersionString"));
		CFStringRef build = (CFStringRef)CFBundleGetValueForInfoDictionaryKey(bundle, CFSTR("CFBundleVersion"));
		packages << QString("%1 (%2.%3)").arg(name)
			.arg(QString::fromCFString(ver))
			.arg(QString::fromCFString(build));
	}
#elif defined(Q_OS_LINUX)
	QProcess p;

	for(const QString &name: names)
	{
		p.start( "dpkg-query", QStringList() << "-W" << "-f=${Version}" << name );
		if( !p.waitForStarted() && p.error() == QProcess::FailedToStart )
		{
			p.start( "rpm", QStringList() << "-q" << "--qf" << "%{VERSION}" << name );
			p.waitForStarted();
		}
		p.waitForFinished();
		if( !p.exitCode() )
		{
			QString ver = QString::fromLocal8Bit( p.readAll().trimmed() );
			if( !ver.isEmpty() )
				packages << packageName( name, ver, withName );
		}
	}
#endif
	return packages;
}

void Common::setAccessibleName( QLabel *widget )
{
	QTextDocument doc;
	doc.setHtml( widget->text() );
	widget->setAccessibleName( doc.toPlainText() );
}
