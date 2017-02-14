/*
 * QDigiDocCommon
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

#include "TokenData.h"

////////////////////////////////////////////////////////////////////////////////// TODO XXXXX #include "Configuration.h"
#include "SslCertificate.h"

#include <QtCore/QDateTime>
#include <QtCore/QHash>
#include <QtCore/QJsonObject>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtNetwork/QSslKey>

class TokenDataPrivate: public QSharedData
{
public:
	QString card;
	QStringList cards, readers;
	QSslCertificate cert;
	TokenData::TokenFlags flags = 0;
};



TokenData::TokenData(): d( new TokenDataPrivate ) {}
TokenData::TokenData( const TokenData &other ): d( other.d ) {}
TokenData::~TokenData() {}

QString TokenData::card() const { return d->card; }
void TokenData::setCard( const QString &card ) { d->card = card; }

QStringList TokenData::cards() const { return d->cards; }
void TokenData::setCards( const QStringList &cards ) { d->cards = cards; }

bool TokenData::cardsOrder( const QString &s1, const QString &s2 )
{
	auto cardsOrderScore = []( QChar c ) -> quint8 {
		switch( c.toLatin1() )
		{
		case 'N': return 6;
		case 'A': return 5;
		case 'P': return 4;
		case 'E': return 3;
		case 'F': return 2;
		case 'B': return 1;
		default: return 0;
		}
	};
	QRegExp r("(\\w{1,2})(\\d{7})");
	if( r.indexIn( s1 ) == -1 )
		return false;
	QStringList cap1 = r.capturedTexts();
	if( r.indexIn( s2 ) == -1 )
		return false;
	QStringList cap2 = r.capturedTexts();
	// new cards to front
	if( cap1[1].size() != cap2[1].size() )
		return cap1[1].size() > cap2[1].size();
	// card type order
	if( cap1[1][0] != cap2[1][0] )
		return cardsOrderScore( cap1[1][0] ) > cardsOrderScore( cap2[1][0] );
	// card version order
	if( cap1[1].size() > 1 && cap2[1].size() > 1 && cap1[1][1] != cap2[1][1] )
		return cap1[1][1] > cap2[1][1];
	// serial number order
	return cap1[2].toUInt() > cap2[2].toUInt();
}

QSslCertificate TokenData::cert() const { return d->cert; }
void TokenData::setCert( const QSslCertificate &cert ) { d->cert = cert; }

void TokenData::clear() { d = new TokenDataPrivate; }

TokenData::TokenFlags TokenData::flags() const { return d->flags; }
void TokenData::setFlag( TokenFlags flag, bool enabled )
{
	if( enabled ) d->flags |= flag;
	else d->flags &= ~flag;
}
void TokenData::setFlags( TokenFlags flags ) { d->flags = flags; }

QStringList TokenData::readers() const { return d->readers; }
void TokenData::setReaders( const QStringList &readers ) { d->readers = readers; }

QString TokenData::toAccessible() const
{
	QString accessible;
	QTextStream s( &accessible );
	SslCertificate c( d->cert );

	if( c.type() & SslCertificate::TempelType )
	{
		s << tr("Company") << " " << c.toString( "CN" ) << " "
			<< tr("Register code") << " " << c.subjectInfo( "serialNumber" );
	}
	else
	{
		s << tr("Name") << " " << c.toString( "GN SN" ) << " "
			<< tr("Personal code") << " " << c.subjectInfo( "serialNumber" );
	}
	s << " " << tr("Card in reader") << " " << d->card;

	bool willExpire = c.expiryDate().toLocalTime() <= QDateTime::currentDateTime().addDays( 105 );
	s << " " << (c.keyUsage().keys().contains( SslCertificate::NonRepudiation ) ?
		tr("Sign certificate is") : tr("Auth certificate is")  );
	if( c.isValid() )
	{
		s << " " << tr("valid");
		if( willExpire )
			s << " " << tr("Your certificates will expire soon");
	}
	else
		s << " " << tr("expired");
	if( d->flags & TokenData::PinLocked )
		s << " " << tr("PIN is locked");

	return accessible;
}

QString TokenData::toHtml() const
{
	QString content;
	QTextStream s( &content );
	SslCertificate c( d->cert );

	s << "<table width=\"100%\"><tr><td>";
	if( c.type() & SslCertificate::TempelType )
	{
		s << tr("Company") << ": <font color=\"black\">"
			<< (c.subjectInfo("O").isEmpty() ? c.toString("CN") : c.toString("O")) << "</font><br />";
		if( !c.subjectInfo("serialNumber").isEmpty() )
		{
			s << tr("Register code") << ": <font color=\"black\">"
				<< c.subjectInfo("serialNumber") << "</font><br />";
		}
	}
	else
	{
		bool breakOnSerial = true;
		if(!c.subjectInfo("GN").isEmpty() && !c.subjectInfo("SN").isEmpty())
		{
			s << tr("Given Names") << ": <font color=\"black\">"
				<< c.toString( "GN" ) << "</font><br />";
			s << tr("Surname") << ": <font color=\"black\">"
				<< c.toString( "SN" ) << "</font><br />";
			breakOnSerial = false;
		}
		else
		{
			s << tr("Name") << ": <font color=\"black\">"
				<< c.toString( "CN" ) << "</font><br />";
		}
		if(!c.subjectInfo( "serialNumber" ).isEmpty())
		{
			s << tr("Personal code") << ": <font color=\"black\">"
				<< c.subjectInfo( "serialNumber" ) << "</font>" << ( breakOnSerial ? "<br />" : "&nbsp;&nbsp;" );
		}
	}
	s << tr("Card in reader") << ": <font color=\"black\">" << d->card << "</font><br />";

	s << (c.keyUsage().keys().contains( SslCertificate::NonRepudiation ) ? tr("Sign certificate is") : tr("Auth certificate is")  ) << " ";
	if( c.isValid() )
	{
		s << "<font color=\"green\">" << tr("valid") << "</font>";
#ifdef CONFIG_URL
		if(Configuration::instance().object().contains("EIDUPDATER-URL") &&
			c.publicKey().length() > 1024 &&
			(c.type() & SslCertificate::EstEidType || c.type() & SslCertificate::DigiIDType) &&
			(!c.validateEncoding() || (Configuration::instance().object().contains("EIDUPDATER-SHA1") && c.signatureAlgorithm() == "sha1WithRSAEncryption")))
		{
			s << ",<br /><font color=\"red\">" << tr("but needs an update.")
				<< "</font> <a href=\"openUtility\"><font color=\"red\">" << tr("Update") << "</font></a>";
		}
		else
#endif
		if(c.expiryDate().toLocalTime() <= QDateTime::currentDateTime().addDays(105))
			s << "<br /><font color=\"red\">" << tr("Your certificates will expire soon") << "</font>";
	}
	else
		s << "<font color=\"red\">" << tr("expired") << "</font>";
	if( d->flags & TokenData::PinLocked )
		s << "<br /><font color=\"red\">" << tr("PIN is locked") << "</font>";

	s << "</td><td align=\"center\" width=\"75\">";
	if(d->flags & TokenData::PinLocked && (c.type() & SslCertificate::EstEidType || c.type() & SslCertificate::DigiIDType))
	{
		s << "<a href=\"openUtility\"><img src=\":/images/warning.png\"><br />"
			"<font color=\"red\">" << tr("Open utility") << "</font></a>";
	}
	else if( c.type() & SslCertificate::TempelType )
		s << "<img src=\":/images/ico_stamp_blue_75.png\">";
	else
		s << "<img src=\":/images/ico_person_blue_75.png\">";
	s << "</td></tr></table>";

	return content;
}

TokenData& TokenData::operator =( const TokenData &other ) { d = other.d; return *this; }

bool TokenData::operator !=( const TokenData &other ) const { return !(operator==(other)); }

bool TokenData::operator ==( const TokenData &other ) const
{
	return d == other.d ||
		( d->card == other.d->card &&
		  d->cards == other.d->cards &&
		  d->readers == other.d->readers &&
		  d->cert == other.d->cert &&
		  d->flags == other.d->flags );
}
