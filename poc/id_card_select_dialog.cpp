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

#include "id_card_select_dialog.h"

#include <QDebug>
#include <QMovie>
#include <QMessageBox>
#include <QTextStream>
#include <QTimer>

#include "common/SslCertificate.h"

namespace ria_tera {

IDCardSelectDialog::IDCardSelectDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    movie.reset(new QMovie(":/images/wait.gif"));
    labelCardInfo->setMovie(movie.data());

    populateGuiFromIDCard();

    btnStart->setEnabled(false);
    connect(btnStart, &QPushButton::clicked, this, &IDCardSelectDialog::startAuthentication);
    connect(btnCancel, &QPushButton::clicked, this, &IDCardSelectDialog::reject);
    QTimer::singleShot(0, this, SLOT(initSmartCard()));
}

IDCardSelectDialog::~IDCardSelectDialog() = default;

void IDCardSelectDialog::onTranslate() {
    populateGuiFromIDCard();
}

void IDCardSelectDialog::showEvent(QShowEvent * event) {
    QDialog::showEvent(event);
    QTimer::singleShot(0, this, SLOT(initSmartCard()));
}

void IDCardSelectDialog::startAuthentication() {
	if (smartCard.isNull())
		return;
	QSmartCard::ErrorType error = smartCard->login();
	if(error == QSmartCard::ErrorType::NoError)
		return accept();
	bufferCardData();
	QString message;
	switch(error) {
	case QSmartCard::ErrorType::CancelError: return;
	case QSmartCard::ErrorType::ValidateError:
		message = tr("Wrong PIN1.");
		break;
	case QSmartCard::ErrorType::BlockedError:
		message = tr("PIN1 is blocked.");
		break;
	case QSmartCard::ErrorType::UnknownError:
		message = tr("Error occurred while verifying PIN.\nPlease check if ID-card is still in the reader.");
		smartCard->logout();
		break;
	default:
		message = tr("Error: ") + QString::number(error); // TODO
		smartCard->logout();
	}
	populateGuiFromIDCard();
	QMessageBox::warning(this, tr("PIN Verification"), message);
}

void IDCardSelectDialog::initSmartCard() {
    if (smartCard.isNull()) {
        smartCard.reset(QSmartCard::create(pdf));
        connect(smartCard.data(), SIGNAL(dataChanged()), this, SLOT(cardDataChanged()), Qt::QueuedConnection);
        connect(comboBoxIDCardSelect, SIGNAL(activated(QString)),
            smartCard.data(), SLOT(selectCard(QString)), Qt::QueuedConnection);
        smartCard->start();
    } else {
        smartCard->logout();
    }
}

void IDCardSelectDialog::cardDataChanged() {
    bufferCardData();
    populateGuiFromIDCard();
}

void IDCardSelectDialog::populateGuiFromIDCard() {
    // see void MainWindow::updateData() from qestidutil
    populateIDCardInfoText(smartCardData);

    btnStart->setEnabled(CertValidity::Valid == validateAuthCert(smartCardData));

    comboBoxIDCardSelect->clear();
    comboBoxIDCardSelect->addItems(smartCardData.cards());
    comboBoxIDCardSelect->setVisible(smartCardData.cards().size() > 1);
    comboBoxIDCardSelect->setCurrentIndex(comboBoxIDCardSelect->findText(smartCardData.card()));
}

static QString formatLine(QString const& title, QString const& value) {
    QString text;
    QTextStream st(&text);

    //st << "<font style = 'font-weight: bold;'>";
    st << "<font style='color: #54859b;'>" << title.toHtmlEscaped() << " </font>";
    st << "<font style='color: black;'>" << value.toHtmlEscaped() << " </font>";
    //st << "</font>";

    return text;
}

void IDCardSelectDialog::populateIDCardInfoText(TokenData const& t) {
    QString cards;
    QTextStream stCards(&cards);

    QString needPinText = tr("NEED_PIN1_FOR_AUTHENTICATION");
    if (!needPinText.trimmed().isEmpty()) {
        stCards << tr("NEED_PIN1_FOR_AUTHENTICATION") << "<br/><br/>\n";
    }

    stCards << tr("%1 cards in the reader(s)").arg(QString::number(smartCardData.cards().size())) << "<br/><br/>";
    labelCardsInReader->setText(cards);

    if (t.isNull()) {
        labelCardInfo->setAlignment(Qt::AlignCenter | Qt::AlignHCenter);
        labelCardInfo->setMovie(movie.data());
        movie->start();
    } else {
        QString text;
        QTextStream st(&text);

        // Card number
        st << "<font style='color: #54859b;'>" // <font style='font-weight: bold;'>
            << tr("Card in reader") << " <font style='color: black;'>"
			<< t.card() << "</font><br />";

        // User data
		QString fname = t.cert().subjectInfo("GN").join(" ");
        fname = fname.trimmed();
		QString surname = t.cert().subjectInfo("SN").join(" ");
		QString userid = t.cert().subjectInfo("serialNumber").join(" ");

        st << formatLine(tr("Given Names:"), fname) << "<br/>";
        st << formatLine(tr("Surname:"), surname) << "<br/>";
        st << formatLine(tr("Personal Code:"), userid) << "<br/>";

        st << "<br/>";

        // Card validity
        QString authValidity;
        CertValidity certValidity = validateAuthCert(t);
        switch (certValidity)
        {
        case CertValidity::Valid:
            authValidity = tr("valid and applicable");
            break;
        case CertValidity::Invalid:
            authValidity = tr("expired");
            break;
        case CertValidity::ValidButBlocked:
            authValidity = tr("valid but blocked");
            break;
        case CertValidity::InvalidAndBlocked:
            authValidity = tr("invalid and blocked");
            break;
        case CertValidity::NullData:
            break;
        default:
            break;
        }

        QString validityColor = ((CertValidity::Valid == certValidity) ? "#509b00" : "#e80303");
        st << tr("Authentication certificate is");
        st << "<font style='color: " << validityColor << ";'> " << authValidity << "</font>";

        st << "<br/><br/>";

        labelCardInfo->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        labelCardInfo->setText(text);
    }
}

void IDCardSelectDialog::bufferCardData() {
    smartCardData = smartCard->dataXXX();
}

IDCardSelectDialog::CertValidity IDCardSelectDialog::validateAuthCert(TokenData const& t) {
    CertValidity res = CertValidity::NullData;
    if (!t.isNull()) {
        res = (SslCertificate(t.cert()).isValid() ? CertValidity::Valid : CertValidity::Invalid);
    }
    return res;
}
}
