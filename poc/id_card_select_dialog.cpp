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
}

IDCardSelectDialog::~IDCardSelectDialog() {}

void IDCardSelectDialog::showEvent(QShowEvent * event) {
    QDialog::showEvent(event);
    QTimer::singleShot(0, this, SLOT(initSmartCard()));
}

void IDCardSelectDialog::startAuthentication() {
    if (smartCard.isNull()) return;
    qDebug() << "ddljadfljlasdfjkl;sdfds 111";
    QSmartCard::ErrorType error = smartCard->login(QSmartCardData::Pin1Type);
    if (QSmartCard::ErrorType::NoError != error) {
        QMessageBox::warning(this, "xxxxxxxxxxxxxxx", tr("Error: ") + QString::number(error)); // TODO
    } else {
        accept();
    }
    qDebug() << "ddljadfljlasdfjkl;sdfds 222";
}

void IDCardSelectDialog::initSmartCard() {
    smartCard.reset(new QSmartCard());
    connect(smartCard.data(), SIGNAL(dataChanged()), this, SLOT(cardDataChanged()));
    connect(comboBoxIDCardSelect, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),
        smartCard.data(), &QSmartCard::selectCard);
    smartCard->start();
}

void IDCardSelectDialog::cardDataChanged() {
    qDebug() << "data changed";
    bufferCardData();
    populateGuiFromIDCard();
}

void IDCardSelectDialog::populateGuiFromIDCard() {
    qDebug() << "aaaa";
    // see void MainWindow::updateData() from qestidutil
    populateIDCardInfoText(smartCardData);

    btnStart->setEnabled(!smartCardData.isNull());

    comboBoxIDCardSelect->clear();
    comboBoxIDCardSelect->addItems(smartCardData.cards());
    comboBoxIDCardSelect->setVisible(smartCardData.cards().size() > 1);
    comboBoxIDCardSelect->setCurrentIndex(comboBoxIDCardSelect->findText(smartCardData.card()));
}

void IDCardSelectDialog::populateIDCardInfoText(QSmartCardData const& t) {
    QString text;
    QTextStream st(&text);

    st << QString::number(smartCardData.cards().size()) + " cards in the reader(s)<br/><br/>";

    if (!t.isNull()) {
        st << "<font style='color: #54859b;'><font style='font-weight: bold; font-size: 16px;'>"
            << tr("Card in reader") << " <font style='color: black;'>"
            << t.data(QSmartCardData::DocumentId).toString() << "</font></font><br />";
        if (t.authCert().type() & SslCertificate::EstEidType) {
            st << tr("This is");
            if (t.isValid())
                st << " <font style='color: #509b00;'>" << tr("valid") << "</font> ";
            else
                st << " <font style='color: #e80303;'>" << tr("expired") << "</font> ";
            st << tr("document") << "<br />";
        } else {
            st << tr("You're using Digital identity card") << "<br />";
        }
        st << tr("Card is valid till") << " <font style='color: black;'>"
            << t.data(QSmartCardData::Expiry).toDateTime().toString("dd. MMMM yyyy") << "</font>";

        st << "<br/><br/>";

        st << t.data(QSmartCardData::FirstName1).toString() << " "
            << t.data(QSmartCardData::FirstName2).toString() << "<br/>"
            << t.data(QSmartCardData::SurName).toString() << "<br/>"
            << t.data(QSmartCardData::Id).toString();
    }

    label->setText(text);
}

void IDCardSelectDialog::bufferCardData() {
qDebug() << "aaaa bl";
    QMutexLocker lock(&smartCard->mutex());
qDebug() << "aaaa al";
    smartCardData = smartCard->data();
}

}