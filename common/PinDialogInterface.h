#pragma once

#include <QSslCertificate>

class PinDialogInterface : public QObject {
    Q_OBJECT
public:
    enum PinFlags
    {
        Pin1Type = 0,
        Pin2Type = 1,
        PinpadFlag = 2,
        PinpadNoProgressFlag = 4,
        Pin1PinpadType = Pin1Type | PinpadFlag,
        Pin2PinpadType = Pin2Type | PinpadFlag
    };

    /// Runs PIN asking process. See PinpadFlag flag passed to PinDialogFactory
    /// When pinpad not present getPin is asked next
    /// \return true if success
    virtual bool execDialog() = 0;
    virtual QByteArray getPin() = 0;
    /// doStartTimer & doFinish are called in that order together with execDialog
    /// after doFinish execDialog should end with false, used only with pinpad
    virtual void doStartTimer() = 0; // ??
    virtual void doFinish(int result) = 0; // ??
};

class PinDialogFactory {
public:
    virtual PinDialogInterface* createPinDialog(PinDialogInterface::PinFlags flags, const QSslCertificate &cert) = 0;
};
