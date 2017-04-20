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

#include "QPCSC.h"
#include "QPCSC_p.h"

#include <QtCore/QDateTime>
#include <QtCore/QLoggingCategory>
#include <QtCore/QStringList>
#include <QtCore/QtEndian>

#include <cstring>

#ifdef Q_OS_WIN
#include <Regstr.h>
#include <Setupapi.h>
#endif

Q_LOGGING_CATEGORY(APDU,"QPCSC.APDU")
Q_LOGGING_CATEGORY(SCard,"QPCSC.SCard")

template < typename Func, typename... Args>
LONG SCCall( const char *file, int line, const char *function, Func func, Args... args)
{
	LONG err = func(args...);
	if(SCard().isDebugEnabled())
		QMessageLogger(file, line, function, SCard().categoryName()).debug()
			<< function << hex << (unsigned long)err;
	return err;
}
#define SC(API, ...) SCCall(__FILE__, __LINE__, "SCard"#API, SCard##API, __VA_ARGS__)

QByteArray QPCSCReaderPrivate::attrib( DWORD id ) const
{
	if(!card)
		return QByteArray();
	DWORD size = 0;
	LONG err = SC(GetAttrib, card, id, nullptr, &size);
	if( err != SCARD_S_SUCCESS || !size )
		return QByteArray();
	QByteArray data( size, 0 );
	err = SC(GetAttrib, card, id, LPBYTE(data.data()), &size);
	if( err != SCARD_S_SUCCESS || !size )
		return QByteArray();
	return data;
}

QHash<DRIVER_FEATURES,quint32> QPCSCReaderPrivate::features()
{
	if(!featuresList.isEmpty())
		return featuresList;
	DWORD size = 0;
	BYTE feature[256];
	LONG rv = SC(Control, card, CM_IOCTL_GET_FEATURE_REQUEST, nullptr, 0, feature, DWORD(sizeof(feature)), &size);
	if(rv != SCARD_S_SUCCESS)
		return featuresList;
	for(unsigned char *p = feature; DWORD(p-feature) < size; )
	{
		int tag = *p++, len = *p++, value = 0;
		for(int i = 0; i < len; ++i)
			value |= *p++ << 8 * i;
		featuresList[DRIVER_FEATURES(tag)] = qFromBigEndian<quint32>(value);
	}
	return featuresList;
}



QPCSC::QPCSC()
	: d( new QPCSCPrivate )
{
	const_cast<QLoggingCategory&>(SCard()).setEnabled(QtDebugMsg, qEnvironmentVariableIsSet("PCSC_DEBUG"));
	const_cast<QLoggingCategory&>(APDU()).setEnabled(QtDebugMsg, qEnvironmentVariableIsSet("APDU_DEBUG"));
	SC(EstablishContext, SCARD_SCOPE_USER, nullptr, nullptr, &d->context);
}

QPCSC::~QPCSC()
{
	if( d->context )
		SC(ReleaseContext, d->context);
	qDeleteAll(d->lock);
	delete d;
}

QStringList QPCSC::drivers() const
{
#ifdef Q_OS_WIN
	HDEVINFO h = SetupDiGetClassDevs( 0, 0, 0, DIGCF_ALLCLASSES | DIGCF_PRESENT );
	if( !h )
		return QStringList();

	SP_DEVINFO_DATA info = { sizeof(SP_DEVINFO_DATA) };
	QStringList list;
	for( DWORD i = 0; SetupDiEnumDeviceInfo( h, i, &info ); i++ )
	{
		DWORD size = 0;
		WCHAR data[1024];

		SetupDiGetDeviceRegistryPropertyW( h, &info,
			SPDRP_CLASS, 0, LPBYTE(data), sizeof(data), &size );
		if( _wcsicmp( data, L"SmartCardReader" ) != 0 )
			continue;

		DWORD conf = 0;
		SetupDiGetDeviceRegistryPropertyW( h, &info,
			SPDRP_CONFIGFLAGS, 0, LPBYTE(&conf), sizeof(conf), &size );
		if( conf & CONFIGFLAG_DISABLED )
			continue;

		SetupDiGetDeviceRegistryPropertyW( h, &info,
			SPDRP_DEVICEDESC, 0, LPBYTE(data), sizeof(data), &size );
		QString name( (QChar*)data );

		SetupDiGetDeviceRegistryPropertyW( h, &info,
			SPDRP_HARDWAREID, 0, LPBYTE(data), sizeof(data), &size );

		list << QString( "%1 (%2)").arg( name, QString( (QChar*)data ) );
	}
	SetupDiDestroyDeviceInfoList( h );

	return list;
#else
	return readers();
#endif
}

QPCSC& QPCSC::instance()
{
	static QPCSC pcsc;
	return pcsc;
}

QStringList QPCSC::readers() const
{
	if( !serviceRunning() )
		return QStringList();

	DWORD size = 0;
	LONG err = SC(ListReaders, d->context, nullptr, nullptr, &size);
	if( err != SCARD_S_SUCCESS || !size )
		return QStringList();

	QByteArray data( size, 0 );
	err = SC(ListReaders, d->context, nullptr, data.data(), &size);
	if( err != SCARD_S_SUCCESS )
		return QStringList();

	QStringList readers = QString::fromLocal8Bit(data, size).split(QChar(0));
	readers.removeAll(QString());
	return readers;
}

bool QPCSC::serviceRunning() const
{
	if(d->context)
		return true;
	SC(EstablishContext, SCARD_SCOPE_USER, nullptr, nullptr, &d->context);
	return d->context;
}



QPCSCReader::QPCSCReader( const QString &reader, QPCSC *parent )
	: d( new QPCSCReaderPrivate )
{
	if(!parent->d->lock.contains(reader))
		parent->d->lock[reader] = new QMutex();
	parent->d->lock[reader]->lock();
	std::memset( &d->state, 0, sizeof(d->state) );
	d->d = parent->d;
	d->reader = reader.toUtf8();
	d->state.szReader = d->reader.constData();
	updateState();
}

QPCSCReader::~QPCSCReader()
{
	disconnect();
	d->d->lock[d->reader]->unlock();
	delete d;
}

QByteArray QPCSCReader::atr() const
{
	return QByteArray::fromRawData( (const char*)d->state.rgbAtr, d->state.cbAtr ).toHex().toUpper();
}

bool QPCSCReader::beginTransaction()
{
	return SC(BeginTransaction, d->card) == SCARD_S_SUCCESS;
}

bool QPCSCReader::connect(Connect connect, Mode mode)
{
	return connectEx(connect, mode) == SCARD_S_SUCCESS;
}

quint32 QPCSCReader::connectEx(Connect connect, Mode mode)
{
	LONG err = SC(Connect, d->d->context, d->state.szReader, connect, mode, &d->card, &d->proto);
	updateState();
	return err;
}

void QPCSCReader::disconnect( Reset reset )
{
	if( d->card )
		SC(Disconnect, d->card, reset);
	d->proto = 0;
	d->card = 0;
	d->featuresList.clear();
	updateState();
}

bool QPCSCReader::endTransaction( Reset reset )
{
	return SC(EndTransaction, d->card, reset) == SCARD_S_SUCCESS;
}

QString QPCSCReader::friendlyName() const
{
	return QString::fromLocal8Bit( d->attrib( SCARD_ATTR_DEVICE_FRIENDLY_NAME_A ) );
}

bool QPCSCReader::isConnected() const
{
	return d->card;
}

bool QPCSCReader::isPinPad() const
{
	if(qEnvironmentVariableIsSet("SMARTCARDPP_NOPINPAD"))
		return false;
	QHash<DRIVER_FEATURES,quint32> features = d->features();
	return features.contains(FEATURE_VERIFY_PIN_DIRECT) || features.contains(FEATURE_VERIFY_PIN_START);
}

bool QPCSCReader::isPresent() const
{
	return d->state.dwEventState & SCARD_STATE_PRESENT;
}

QString QPCSCReader::name() const
{
	return QString::fromLocal8Bit( d->reader );
}

QHash<QPCSCReader::Properties, int> QPCSCReader::properties() const
{
	QHash<Properties,int> properties;
	if( DWORD ioctl = d->features().value(FEATURE_GET_TLV_PROPERTIES) )
	{
		DWORD size = 0;
		BYTE recv[256];
		DWORD rv = SC(Control, d->card, ioctl, nullptr, 0, recv, DWORD(sizeof(recv)), &size);
		if(rv != SCARD_S_SUCCESS)
			return properties;
		for(unsigned char *p = recv; DWORD(p-recv) < size; )
		{
			int tag = *p++, len = *p++, value = 0;
			for(int i = 0; i < len; ++i)
				value |= *p++ << 8 * i;
			properties[Properties(tag)] = value;
		}
	}
	return properties;
}

int QPCSCReader::protocol() const
{
	return d->proto;
}

bool QPCSCReader::reconnect( Reset reset, Mode mode )
{
	if( !d->card )
		return false;
	LONG err = SC(Reconnect, d->card, SCARD_SHARE_SHARED, mode, reset, &d->proto);
	updateState();
	return err == SCARD_S_SUCCESS;
}

QStringList QPCSCReader::state() const
{
	QStringList result;
#define STATE(X) if( d->state.dwEventState & SCARD_STATE_##X ) result << #X
	STATE(IGNORE);
	STATE(CHANGED);
	STATE(UNKNOWN);
	STATE(UNAVAILABLE);
	STATE(EMPTY);
	STATE(PRESENT);
	STATE(ATRMATCH);
	STATE(EXCLUSIVE);
	STATE(INUSE);
	STATE(MUTE);
	return result;
}

QPCSCReader::Result QPCSCReader::transfer( const char *cmd, int size ) const
{
	return transfer( QByteArray::fromRawData( cmd, size ) );
}

QPCSCReader::Result QPCSCReader::transfer( const QByteArray &apdu ) const
{
	static const SCARD_IO_REQUEST T0 = { 1, 8 };
	static const SCARD_IO_REQUEST T1 = { 2, 8 };
	QByteArray data( 1024, 0 );
	DWORD size = data.size();

	qCDebug(APDU).nospace() << "T" << qint8(d->proto == SCARD_PROTOCOL_RAW ? -1 : d->proto - 1)
		<< "> " << apdu.toHex().constData();
	DWORD ret = SC(Transmit, d->card, d->proto == SCARD_PROTOCOL_T0 ? &T0 : &T1,
		LPCBYTE(apdu.constData()), apdu.size(), nullptr, LPBYTE(data.data()), &size);
	if( ret != SCARD_S_SUCCESS )
		return Result({ QByteArray(), QByteArray(), ret });

	Result result = { data.mid( size-2, 2 ), data.left( size - 2 ), ret };
	qCDebug(APDU).nospace() << "T" << qint8(d->proto == SCARD_PROTOCOL_RAW ? -1 : d->proto - 1)
		<< "< " << result.SW.toHex().constData();
	if(!result.data.isEmpty()) qCDebug(APDU).nospace() << data.left(size).toHex().constData();

	if( result.SW.at( 0 ) == 0x61 )
	{
		QByteArray cmd( "\x00\xC0\x00\x00\x00", 5 );
		cmd[4] = data.at( size-1 );
		Result result2 = transfer( cmd );
		result2.data.prepend(result.data);
		return result2;
	}
	return result;
}

QPCSCReader::Result QPCSCReader::transferCTL( const QByteArray &apdu, bool verify, quint8 lang, quint8 minlen ) const
{
	bool display = false;
	QHash<DRIVER_FEATURES,quint32> features = d->features();
	if( DWORD ioctl = features.value(FEATURE_IFD_PIN_PROPERTIES) )
	{
		DWORD size = 0;
		BYTE recv[256];
		DWORD rv = SC(Control, d->card, ioctl, nullptr, 0, recv, DWORD(sizeof(recv)), &size);
		if( rv == SCARD_S_SUCCESS )
		{
			PIN_PROPERTIES_STRUCTURE *caps = (PIN_PROPERTIES_STRUCTURE *)recv;
			display = caps->wLcdLayout > 0;
		}
	}

	#define SET() \
		data->bTimerOut = 30; \
		data->bTimerOut2 = 30; \
		data->bmFormatString = 0x02; \
		data->bmPINBlockString = 0x00; \
		data->bmPINLengthFormat = 0x00; \
		data->wPINMaxExtraDigit = (minlen << 8) + 12; \
		data->bEntryValidationCondition = 0x02; \
		data->wLangId = lang; \
		data->bTeoPrologue[0] = 0x00; \
		data->bTeoPrologue[1] = 0x00; \
		data->bTeoPrologue[2] = 0x00

	QByteArray cmd( 255, 0 );
	if( verify )
	{
		PIN_VERIFY_STRUCTURE *data = (PIN_VERIFY_STRUCTURE*)cmd.data();
		SET();
		data->bNumberMessage = display ? 0xFF: 0x00;
		data->bMsgIndex = 0x00;
		data->ulDataLength = apdu.size();
		cmd.resize( sizeof(PIN_VERIFY_STRUCTURE) - 1 );
	}
	else
	{
		PIN_MODIFY_STRUCTURE *data = (PIN_MODIFY_STRUCTURE*)cmd.data();
		SET();
		data->bNumberMessage = display ? 0x03: 0x00;
		data->bInsertionOffsetOld = 0x00;
		data->bInsertionOffsetNew = 0x00;
		data->bConfirmPIN = 0x03;
		data->bMsgIndex1 = 0x00;
		data->bMsgIndex2 = 0x01;
		data->bMsgIndex3 = 0x02;
		data->ulDataLength = apdu.size();
		cmd.resize( sizeof(PIN_MODIFY_STRUCTURE) - 1 );
	}
	cmd += apdu;

	DWORD ioctl = features.value( verify ? FEATURE_VERIFY_PIN_START : FEATURE_MODIFY_PIN_START );
	if( !ioctl )
		ioctl = features.value( verify ? FEATURE_VERIFY_PIN_DIRECT : FEATURE_MODIFY_PIN_DIRECT );

	qCDebug(APDU).nospace() << "T" << qint8(d->proto == SCARD_PROTOCOL_RAW ? -1 : d->proto - 1)
		<< "> " << apdu.toHex().constData();
	qCDebug(APDU).nospace() << "CTL" << "> " << cmd.toHex().constData();
	QByteArray data( 255 + 3, 0 );
	DWORD size = data.size();
	DWORD err = SC(Control, d->card, ioctl, cmd.constData(), cmd.size(), LPVOID(data.data()), data.size(), &size);

	if( DWORD finish = features.value( verify ? FEATURE_VERIFY_PIN_FINISH : FEATURE_MODIFY_PIN_FINISH ) )
	{
		size = data.size();
		err = SC(Control, d->card, finish, nullptr, 0, LPVOID(data.data()), data.size(), &size);
	}

	Result result = { data.mid( size-2, 2 ), data.left( size - 2 ), err };
	qCDebug(APDU).nospace() << "T" << qint8(d->proto == SCARD_PROTOCOL_RAW ? -1 : d->proto - 1)
		<< "< " << result.SW.toHex().constData();
	if(!result.data.isEmpty()) qCDebug(APDU).nospace() << data.left(size).toHex().constData();
	return result;
}

/// At the moment only used as keep-alive for SC transaction
void QPCSCReader::getstatus() {
    char name[100];
    DWORD len = sizeof name;
    DWORD dwState, dwProtocol;
    // unsigned char atr[MAX_ATR_SIZE];
    // DWORD atr_len = sizeof atr;
    DWORD ret = SC(Status, d->card, name, &len, &dwState, &dwProtocol, nullptr, nullptr);
    // if (err != SCARD_S_SUCCESS) {}
}

bool QPCSCReader::updateState( quint32 msec )
{
	if(!d->d->context)
		return false;
	d->state.dwCurrentState = d->state.dwEventState; //(currentReaderCount << 16)
	DWORD err = SC(GetStatusChange, d->d->context, msec, &d->state, 1); //INFINITE
	switch(err) {
	case SCARD_S_SUCCESS: return true;
	case SCARD_E_TIMEOUT: return msec == 0;
	default: return false;
	}
}
