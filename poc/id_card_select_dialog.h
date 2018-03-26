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

#pragma once

#include <QDialog>
#include <QScopedPointer>

#include "ui_IDCardSelectionDialog.h"

#include "common/PinDialog.h"
#include "common/QSmartCard.h"

namespace ria_tera {

class IDCardSelectDialog : public QDialog, public Ui::IDCardSelectionDialog {
    Q_OBJECT

public:
    enum CertValidity
    {
        Valid,
        Invalid,
        ValidButBlocked,
        InvalidAndBlocked,
        NullData
    };

    explicit IDCardSelectDialog(QWidget *parent = 0);
    virtual ~IDCardSelectDialog();

    void onTranslate(); // TODO
protected:
    void showEvent(QShowEvent * event);

private slots:
    void initSmartCard();
    void cardDataChanged();

    void startAuthentication();
private:
    void bufferCardData();
    void populateGuiFromIDCard();
    void populateIDCardInfoText(const TokenData &t);
    CertValidity validateAuthCert(TokenData const& t);

    QScopedPointer<QMovie> movie;

public: // TODO
    PinDialogGUIFactory pdf;
    QSharedPointer<QSmartCard> smartCard;
    TokenData smartCardData;
};

}
