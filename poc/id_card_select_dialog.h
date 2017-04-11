#pragma once

#include <QDialog>

#include "ui_IDCardSelectionDialog.h"

#include "common/PinDialog.h"
#include "common/QSmartCard.h"

namespace ria_tera {

class IDCardSelectDialog : public QDialog, public Ui::IDCardSelectionDialog {
    Q_OBJECT

public:
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
    void populateIDCardInfoText(QSmartCardData const& t);

public: // TODO
    PinDialogGUIFactory pdf;
    QSharedPointer<QSmartCard> smartCard;
    QSmartCardData smartCardData;
};

}
