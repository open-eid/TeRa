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

#include "PinDialog.h"

////////////////////////////////////////////////////////////////////////////// TODO??? #include "Common.h"
#include "SslCertificate.h"

#include <QtCore/QTimeLine>
#include <QtGui/QRegExpValidator>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

#include <QtWidgets/QApplication>

PinDialogInterface* PinDialogGUIFactory::createPinDialog(PinDialogInterface::PinFlags flags, const QSslCertificate &cert) {
    return new PinDialog(flags, cert, 0, qApp->activeWindow());
}

bool PinDialog::execDialog() {
    return QDialog::Accepted == d.exec();
}

QByteArray PinDialog::getPin() {
    return text().toUtf8();
}

void PinDialog::doStartTimer() {
    Q_EMIT startTimer();
}

void PinDialog::doFinish(int result) {
    Q_EMIT finish(0);
}

PinDialog::PinDialog( PinDialogInterface::PinFlags flags, const TokenData &t, QWidget *parent )
:	d( parent )
{
	SslCertificate c = t.cert();
	init( flags, c.toString( c.showCN() ? "CN serialNumber" : "GN SN serialNumber" ), t.flags() );
}

PinDialog::PinDialog( PinDialogInterface::PinFlags flags, const QSslCertificate &cert, TokenData::TokenFlags token, QWidget *parent)
:	d( parent )
{
	SslCertificate c = cert;
	init( flags, c.toString( c.showCN() ? "CN serialNumber" : "GN SN serialNumber" ), token );
}

PinDialog::PinDialog( PinDialogInterface::PinFlags flags, const QString &title, TokenData::TokenFlags token, QWidget *parent )
	: d(parent)
{
	init( flags, title, token );
}

void PinDialog::init( PinDialogInterface::PinFlags flags, const QString &title, TokenData::TokenFlags token )
{
	connect(this, &PinDialog::finish, &d, &QDialog::done);
	d.setMinimumWidth( 350 );
    d.setWindowModality(Qt::ApplicationModal);

	QLabel *label = new QLabel( &d );
	QVBoxLayout *l = new QVBoxLayout( &d );
	l->addWidget( label );

	QString _title = title;
	QString text;

	if( token & TokenData::PinFinalTry )
		text += "<font color='red'><b>" + tr("PIN will be locked next failed attempt") + "</b></font><br />";
	else if( token & TokenData::PinCountLow )
		text += "<font color='red'><b>" + tr("PIN has been entered incorrectly one time") + "</b></font><br />";

	text += QString( "<b>%1</b><br />" ).arg( title );
	if( flags & Pin2Type )
	{
		_title = tr("Signing") + " - " + title;
		QString t = flags & PinpadFlag ?
			tr("For using sign certificate enter PIN2 at the reader") :
			tr("For using sign certificate enter PIN2");
		text += tr("Selected action requires sign certificate.") + "<br />" + t;
		regexp.setPattern( "\\d{5,12}" );
	}
	else
	{
		_title = tr("Authentication") + " - " + title;
		QString t = flags & PinpadFlag ?
			tr("For using authentication certificate enter PIN1 at the reader") :
			tr("For using authentication certificate enter PIN1");
		text += tr("Selected action requires authentication certificate.") + "<br />" + t;
		regexp.setPattern( "\\d{4,12}" );
	}
	d.setWindowTitle( _title );
	label->setText( text );
//////////////////////////////////////////////////////////////////////////////// TODO XXXXXXXXXXXXXXXXXXXXXXXXXX Common::setAccessibleName( label );

	if( flags & PinpadFlag )
	{
		d.setWindowFlags( (d.windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint );
		QProgressBar *progress = new QProgressBar( &d );
		progress->setRange( 0, 30 );
		progress->setValue( progress->maximum() );
		progress->setTextVisible( false );
		l->addWidget( progress );
		QTimeLine *statusTimer = new QTimeLine( progress->maximum() * 1000, this );
		statusTimer->setCurveShape( QTimeLine::LinearCurve );
		statusTimer->setFrameRange( progress->maximum(), progress->minimum() );
		connect( statusTimer, SIGNAL(frameChanged(int)), progress, SLOT(setValue(int)) );
		connect( this, SIGNAL(startTimer()), statusTimer, SLOT(start()) );
	}
	else if( !(flags & PinpadNoProgressFlag) )
	{
        m_text = new QLineEdit( &d );
		m_text->setEchoMode( QLineEdit::Password );
		m_text->setFocus();
		m_text->setValidator( new QRegExpValidator( regexp, m_text ) );
		m_text->setMaxLength( 12 );
		connect( m_text, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)) );
		l->addWidget( m_text );
		label->setBuddy( m_text );

		QDialogButtonBox *buttons = new QDialogButtonBox(
			QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, &d );
		ok = buttons->button( QDialogButtonBox::Ok );
		ok->setAutoDefault( true );
		connect( buttons, SIGNAL(accepted()), &d, SLOT(accept()) );
		connect( buttons, SIGNAL(rejected()), &d, SLOT(reject()) );
		l->addWidget( buttons );

		textEdited( QString() );
	}
}

QString PinDialog::text() const { return m_text->text(); }

void PinDialog::textEdited( const QString &text )
{ ok->setEnabled( regexp.exactMatch( text ) ); }
