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

#include "Configuration.h"

#include <QApplication>
//#include "Common.h"
//#include "QPCSC.h"
#include "Settings.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QStandardPaths>
#include <QtCore/QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QMessageBox>

#include <QProcess>

#include <openssl/err.h>
#include <openssl/pem.h>

#include "poc/logging.h"

class ConfigurationPrivate
{
public:
#ifdef LAST_CHECK_DAYS
	ConfigurationPrivate(): s(qApp->applicationName()) {}
#endif

	void initCache(bool clear);
	static bool lessThanVersion( const QString &current, const QString &available );
	void setData(const QByteArray &_data)
	{
		data = _data;
		dataobject = QJsonDocument::fromJson(data).object();
		Settings s2(QSettings::SystemScope);
		for(const QString &key: s2.childKeys())
		{
			if(dataobject.contains(key))
			{
				QVariant value = s2.value(key);
				switch(value.type())
				{
				case QVariant::String:
					dataobject[key] = QJsonValue(value.toString()); break;
				case QVariant::StringList:
					dataobject[key] = QJsonValue(QJsonArray::fromStringList(value.toStringList())); break;
				default: break;
				}
			}
		}
	}
	bool validate(const QByteArray &data, const QByteArray &signature) const;

#ifndef NO_CACHE
#if QT_VERSION >= QT_VERSION_CHECK(5,4,0)
	QString cache = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/";
#else
	QString cache = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/";
#endif
#endif
	QByteArray data, signature, tmpsignature;
	QJsonObject dataobject;
	QUrl rsaurl, url = QUrl(CONFIG_URL);
	RSA *rsa = nullptr;
	QNetworkRequest req;
	QNetworkAccessManager *net = nullptr;
	QList<QNetworkReply*> requestcache;
#ifdef LAST_CHECK_DAYS
	Settings s;
#endif
};

void ConfigurationPrivate::initCache(bool clear)
{
#ifndef NO_CACHE
	// Signature
	QFile f(cache + rsaurl.fileName());
	if(clear && f.exists())
		f.remove();
	if(!f.exists())
	{
		f.copy(":/config.rsa", f.fileName());
		f.setPermissions(QFile::Permissions(0x6444));
	}
	if(f.open(QFile::ReadOnly))
		signature = QByteArray::fromBase64(f.readAll());
	f.close();

	// Config
	f.setFileName(cache + url.fileName());
	if(clear && f.exists())
		f.remove();
	if(!f.exists())
	{
		f.copy(":/config.json", f.fileName());
		f.setPermissions(QFile::Permissions(0x6444));
	}
	if(f.open(QFile::ReadOnly))
		setData(f.readAll());
	f.close();
#else
	// Signature
	QFile f(":/config.rsa");
	if(f.open(QFile::ReadOnly))
		signature = QByteArray::fromBase64(f.readAll());
	f.close();

	// Config
	f.setFileName(":/config.json");
	if(f.open(QFile::ReadOnly))
		setData(f.readAll());
	f.close();
#endif
}

bool ConfigurationPrivate::lessThanVersion( const QString &current, const QString &available )
{
	QStringList curList = current.split('.');
	QStringList avaList = available.split('.');
	for( int i = 0; i < std::max<int>(curList.size(), avaList.size()); ++i )
	{
		bool curconv = false, avaconv = false;
		unsigned int cur = curList.value(i).toUInt( &curconv );
		unsigned int ava = avaList.value(i).toUInt( &avaconv );
		if( curconv && avaconv )
		{
			if( cur != ava )
				return cur < ava;
		}
		else
		{
			int status = QString::localeAwareCompare( curList.value(i), avaList.value(i) );
			if( status != 0 )
				return status < 0;
		}
	}
	return false;
}

bool ConfigurationPrivate::validate(const QByteArray &data, const QByteArray &signature) const
{
	if(!rsa || data.isEmpty())
		return false;

	QByteArray digest(RSA_size(rsa), 0);
	int size = RSA_public_decrypt(signature.size(), (const unsigned char*)signature.constData(),
		(unsigned char*)digest.data(), rsa, RSA_PKCS1_PADDING);
	digest.resize(std::max(size, 0));

	static const QByteArray SHA1_OID = QByteArray::fromHex("3021300906052b0e03021a05000414");
	static const QByteArray SHA224_OID = QByteArray::fromHex("302d300d06096086480165030402040500041c");
	static const QByteArray SHA256_OID = QByteArray::fromHex("3031300d060960864801650304020105000420");
	static const QByteArray SHA384_OID = QByteArray::fromHex("3041300d060960864801650304020205000430");
	static const QByteArray SHA512_OID = QByteArray::fromHex("3051300d060960864801650304020305000440");
	if(digest.startsWith(SHA1_OID))
	{
		if(!digest.endsWith(QCryptographicHash::hash(data, QCryptographicHash::Sha1)))
			return false;
	}
	else if(digest.startsWith(SHA224_OID))
	{
		if(!digest.endsWith(QCryptographicHash::hash(data, QCryptographicHash::Sha224)))
			return false;
	}
	else if(digest.startsWith(SHA256_OID))
	{
		if(!digest.endsWith(QCryptographicHash::hash(data, QCryptographicHash::Sha256)))
			return false;
	}
	else if(digest.startsWith(SHA384_OID))
	{
		if(!digest.endsWith(QCryptographicHash::hash(data, QCryptographicHash::Sha384)))
			return false;
	}
	else if(digest.startsWith(SHA512_OID))
	{
		if(!digest.endsWith(QCryptographicHash::hash(data, QCryptographicHash::Sha512)))
			return false;
	}
	else
		return false;

	QJsonObject obj = QJsonDocument::fromJson(data).object().value("META-INF").toObject();
	return QDateTime::currentDateTimeUtc() > QDateTime::fromString(obj.value("DATE").toString(), "yyyyMMddHHmmss'Z'");
}

////////////////////////////////////////////////////////////////////////

#if defined(Q_OS_WIN)
#include <QtCore/QLibrary>
#include <qt_windows.h>
#elif defined(Q_OS_MAC)
#include <QtCore/QXmlStreamReader>
#include <sys/utsname.h>
#include <CoreFoundation/CFBundle.h>
#endif


QString applicationOs()
{
#if defined(Q_OS_LINUX)
    QProcess p;
    p.start("lsb_release", QStringList() << "-s" << "-d");
    p.waitForFinished();
    return QString::fromLocal8Bit(p.readAll().trimmed());
#elif defined(Q_OS_MAC)
    struct utsname unameData;
    uname(&unameData);
    QFile f("/System/Library/CoreServices/SystemVersion.plist");
    if (f.open(QFile::ReadOnly))
    {
        QXmlStreamReader xml(&f);
        while (xml.readNext() != QXmlStreamReader::Invalid)
        {
            if (!xml.isStartElement() || xml.name() != "key" || xml.readElementText() != "ProductVersion")
                continue;
            xml.readNextStartElement();
            return QString("Mac OS %1 (%2/%3)")
                .arg(xml.readElementText()).arg(QSysInfo::WordSize).arg(unameData.machine);
        }
    }
#elif defined(Q_OS_WIN)
    OSVERSIONINFOEX osvi = { sizeof(OSVERSIONINFOEX) };
    if (GetVersionEx((OSVERSIONINFO *)&osvi))
    {
        bool workstation = osvi.wProductType == VER_NT_WORKSTATION;
        SYSTEM_INFO si;
        typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
        if (PGNSI pGNSI = PGNSI(QLibrary("kernel32").resolve("GetNativeSystemInfo")))
            pGNSI(&si);
        else
            GetSystemInfo(&si);
        QString os;
        switch ((osvi.dwMajorVersion << 8) + osvi.dwMinorVersion)
        {
        case 0x0500: os = workstation ? "2000 Professional" : "2000 Server"; break;
        case 0x0501: os = osvi.wSuiteMask & VER_SUITE_PERSONAL ? "XP Home" : "XP Professional"; break;
        case 0x0502:
            if (GetSystemMetrics(SM_SERVERR2))
                os = "Server 2003 R2";
            else if (workstation && si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
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
        QString extversion((const QChar*)osvi.szCSDVersion);
        return QString("Windows %1 %2(%3 bit)").arg(os)
            .arg(extversion.isEmpty() ? "" : extversion + " ")
            .arg(si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? "64" : "32");
    }
    else
    {
        switch (QSysInfo::WindowsVersion)
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

    return "Unknown OS"; // TODO return tr("Unknown OS");
}

///////////////////////////////////////////////////////////////////////////



Configuration::Configuration(QObject *parent)
	: QObject(parent)
	, d(new ConfigurationPrivate)
{
	Q_INIT_RESOURCE(config);

#ifndef NO_CACHE
	if(!QDir().exists(d->cache))
		QDir().mkpath(d->cache);
#endif
	d->rsaurl = QString("%1%2.rsa")
		.arg(d->url.adjusted(QUrl::RemoveFilename).toString())
		.arg(QFileInfo(d->url.fileName()).baseName());
	d->req.setRawHeader("User-Agent", QString("%1/%2 (%3) Lang: %4 Devices: %5")
		.arg(qApp->applicationName(), qApp->applicationVersion(),
			 applicationOs(), Settings().language(), QString()).toUtf8()); // QPCSC::instance().drivers().join("/")
	d->net = new QNetworkAccessManager(this);
	connect(d->net, &QNetworkAccessManager::sslErrors,
			[=](QNetworkReply *reply, const QList<QSslError> &errors){
		reply->ignoreSslErrors(errors);
	});
	connect(d->net, &QNetworkAccessManager::finished, [=](QNetworkReply *reply){
		d->requestcache.removeAll(reply);
		switch(reply->error())
		{
		case QNetworkReply::NoError:
			if(reply->url() == d->rsaurl)
			{
				d->tmpsignature = QByteArray::fromBase64(reply->readAll());
				if(d->validate(d->data, d->tmpsignature))
				{
#ifdef LAST_CHECK_DAYS
					d->s.setValue("LastCheck", QDate::currentDate().toString("yyyyMMdd"));
#endif
					Q_EMIT finished(false, QString());
					break;
				}
				else
					TERA_LOG(debug) << "Remote signature does not match, downloading new configuration";
				sendRequest(d->url);
			}
			else if(reply->url() == d->url)
			{
				QByteArray data = reply->readAll();
				if(!d->validate(data, d->tmpsignature))
				{
					qWarning() << "Remote configuration is invalid";
					Q_EMIT finished(false, tr("The configuration file located on the server cannot be validated."));
					break;
				}

				QJsonObject obj = QJsonDocument::fromJson(data).object().value("META-INF").toObject();
				QJsonObject old = object().value("META-INF").toObject();
				if(old.value("SERIAL").toInt() > obj.value("SERIAL").toInt())
				{
					qWarning() << "Remote serial is smaller than current";
					Q_EMIT finished(false, tr("Your computer's configuration file is later than the server has."));
					break;
				}

				TERA_LOG(debug) << "Writing new configuration";
				d->setData(data);
				d->signature = d->tmpsignature.toBase64();
#ifndef NO_CACHE
				QFile f(d->cache + d->url.fileName());
				if(f.exists())
					f.remove();
				if(f.open(QFile::WriteOnly))
					f.write(d->data);
				f.close();

				f.setFileName(d->cache + d->rsaurl.fileName());
				if(f.exists())
					f.remove();
				if(f.open(QFile::WriteOnly))
					f.write(d->signature);
				f.close();
#endif
#ifdef LAST_CHECK_DAYS
				d->s.setValue("LastCheck", QDate::currentDate().toString("yyyyMMdd"));
#endif
				Q_EMIT finished(true, QString());
			}
			break;
		default:
			Q_EMIT finished(false, reply->errorString());
			break;
		}
		reply->deleteLater();
	});

	QFile f(":/config.pub");
	if(!f.open(QFile::ReadOnly))
	{
		qWarning() << "Failed to read public key";
		return;
	}

	QByteArray key = f.readAll();
	BIO *bio = BIO_new_mem_buf((void*)key.constData(), key.size());
	if(!bio)
	{
		qWarning() << "Failed to parse public key";
		return;
	}

	d->rsa = PEM_read_bio_RSAPublicKey(bio, nullptr, nullptr, nullptr);
	BIO_free(bio);
	if(!d->rsa)
	{
		qWarning() << "Failed to parse public key";
		return;
	}

	d->initCache(false);
	if(!d->validate(d->data, d->signature))
	{
		qWarning() << "Config siganture is invalid, clearing cache";
		d->initCache(true);
	}
	else
	{
		int serial = object().value("META-INF").toObject().value("SERIAL").toInt();
		TERA_LOG(debug) << "Chache configuration serial:" << serial;
		QFile embedConf(":/config.json");
		if(embedConf.open(QFile::ReadOnly))
		{
			QJsonObject obj = QJsonDocument::fromJson(embedConf.readAll()).object();
			int bundledSerial = obj.value("META-INF").toObject().value("SERIAL").toInt();
			TERA_LOG(debug) << "Bundled configuration serial:" << bundledSerial;
			if(serial < bundledSerial)
			{
				qWarning() << "Bundled configuration is recent than cache, resetting cache";
				d->initCache(true);
			}
		}
	}

	Q_EMIT finished(true, QString());

#ifdef LAST_CHECK_DAYS
	if(d->s.value("LastCheck").isNull())
		d->s.setValue("LastCheck", QDate::currentDate().toString("yyyyMMdd"));
	QDate lastCheck = QDate::fromString(d->s.value("LastCheck").toString(), "yyyyMMdd");
	if(lastCheck < QDate::currentDate().addDays(-LAST_CHECK_DAYS))
		update();
#endif
}

Configuration::~Configuration()
{
	if(d->rsa)
		RSA_free(d->rsa);
	delete d;
}

void Configuration::checkVersion(const QString &name)
{
#if 0
	if(ConfigurationPrivate::lessThanVersion(qApp->applicationVersion(), object()[name+"-SUPPORTED"].toString()))
		QMessageBox::warning(qApp->activeWindow(), tr("Update is available"),
			tr("Your ID-software has expired. To download the latest software version, go to the "
				"<a href=\"http://installer.id.ee/?lang=eng\">id.ee</a> website. "
				"Mac OS X users can download the latest ID-software version from the "
				"<a href=\"http://appstore.com/mac/ria\">Mac App Store</a>."));

	connect(this, &Configuration::finished, [=](bool changed, const QString &){
		if(changed && ConfigurationPrivate::lessThanVersion(qApp->applicationVersion(), object()[name+"-LATEST"].toString()))
			QMessageBox::information(qApp->activeWindow(), tr("Update is available"),
				tr("An ID-software update has been found. To download the update, go to the "
					"<a href=\"http://installer.id.ee/?lang=eng\">id.ee</a> website. "
					"Mac OS X users can download the update from the "
					"<a href=\"http://appstore.com/mac/ria\">Mac App Store</a>."));
	});
#endif
}

Configuration& Configuration::instance()
{
	static Configuration conf;
	return conf;
}

QJsonObject Configuration::object() const
{
	return d->dataobject;
}

void Configuration::sendRequest(const QUrl &url)
{
	d->req.setUrl(url);
	QNetworkReply *reply = d->net->get(d->req);
	d->requestcache << reply;
	QTimer *timer = new QTimer(this);
	timer->setSingleShot(true);
	connect(timer, &QTimer::timeout, [=]{
		timer->deleteLater();
		if(!d->requestcache.contains(reply))
			return;
		d->requestcache.removeAll(reply);
		reply->deleteLater();
		TERA_LOG(debug) << "Request timed out";
		Q_EMIT finished(false, tr("Request timed out"));
	});
	timer->start(30*1000);
}

void Configuration::update()
{
	sendRequest(d->rsaurl);
}
