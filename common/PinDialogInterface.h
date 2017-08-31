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
