#include "id_card_select_dialog.h"

#include <QDebug>
#include <QMessageBox>
#include <QTextStream>
#include <QTimer>

#include "common/SslCertificate.h"

namespace ria_tera {

IDCardSelectDialog::IDCardSelectDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    populateGuiFromIDCard();

    btnStart->setEnabled(false);
    connect(btnStart, SIGNAL(clicked(bool)), this, SLOT(startAuthentication()));
    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    QTimer::singleShot(0, this, SLOT(initSmartCard()));
}

IDCardSelectDialog::~IDCardSelectDialog() {}

void IDCardSelectDialog::onTranslate() {
    populateGuiFromIDCard();
}

void IDCardSelectDialog::showEvent(QShowEvent * event) {
    QDialog::showEvent(event);
    QTimer::singleShot(0, this, SLOT(initSmartCard()));
}

void IDCardSelectDialog::startAuthentication() {
    if (smartCard.isNull()) return;
    QSmartCard::ErrorType error = smartCard->login(QSmartCardData::Pin1Type);
    if (QSmartCard::ErrorType::NoError != error) {
        bufferCardData();

        if (QSmartCard::ErrorType::CancelError != error) {
            QString title = tr("PIN Verification");
            QString message;

            if (QSmartCard::ErrorType::ValidateError == error) {
                int retryCount = smartCardData.retryCount(QSmartCardData::PinType::Pin1Type);
                message = tr("Wrong PIN1. %1 retries left").arg(QString::number(retryCount));
            } else if (QSmartCard::ErrorType::BlockedError == error) {
                message = tr("PIN1 is blocked.");
            } else if (QSmartCard::ErrorType::UnknownError == error) {
                message = tr("Error occurred while verifying PIN.\nPlease check if ID-card is still in the reader.");
                smartCard->logout();
            } else {
                message = tr("Error: ") + QString::number(error); // TODO
                smartCard->logout();
            }
            populateGuiFromIDCard();
            QMessageBox::warning(this, title, message);
        }
    } else {
        accept();
    }
}

void IDCardSelectDialog::initSmartCard() {
    if (smartCard.isNull()) {
        smartCard.reset(new QSmartCard(pdf));
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

void IDCardSelectDialog::populateIDCardInfoText(QSmartCardData const& t) {
    QString text;
    QTextStream st(&text);

    QString needPinText = tr("NEED_PIN1_FOR_AUTHENTICATION");
    if (!needPinText.trimmed().isEmpty()) {
        st << tr("NEED_PIN1_FOR_AUTHENTICATION") << "<br/><br/>\n";
    }

    st << tr("%1 cards in the reader(s)").arg(QString::number(smartCardData.cards().size())) << "<br/><br/>";

    if (!t.isNull()) {
        // Card number
        st << "<font style='color: #54859b;'>" // <font style='font-weight: bold;'>
            << tr("Card in reader") << " <font style='color: black;'>"
            << t.data(QSmartCardData::DocumentId).toString() << "</font><br />";

        // User data
        QString fname = t.data(QSmartCardData::FirstName1).toString() + " " + t.data(QSmartCardData::FirstName2).toString();
        fname = fname.trimmed();
        QString surname = t.data(QSmartCardData::SurName).toString();
        QString userid = t.data(QSmartCardData::Id).toString();

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

        int retryCount = smartCardData.retryCount(QSmartCardData::PinType::Pin1Type);
        if (retryCount < 3) {
            st << tr("%1 retries left for PIN1").arg(QString::number(retryCount));
        }
    }

    label->setText(text);
}

void IDCardSelectDialog::bufferCardData() {
    smartCardData = smartCard->dataXXX();
}

IDCardSelectDialog::CertValidity IDCardSelectDialog::validateAuthCert(QSmartCardData const& t) {
    CertValidity res = CertValidity::NullData;
    if (!t.isNull()) {
        if (t.retryCount(QSmartCardData::Pin1Type) == 0) {
            res = (t.authCert().isValid() ? CertValidity::ValidButBlocked : CertValidity::InvalidAndBlocked);
        }
        else {
            res = (t.authCert().isValid() ? CertValidity::Valid : CertValidity::Invalid);
        }
    }
    return res;
}
}