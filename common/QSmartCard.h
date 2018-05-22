/*
 * QEstEidUtil
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

#include <QThread>

#include <common/PinDialogInterface.h>

class PinDialogFactory;
class TokenData;

class QSmartCard: public QThread
{
    Q_OBJECT
public:
    enum ErrorType
    {
        NoError,
        CancelError,
        ValidateError,
        BlockedError,
        UnknownError,
    };

    static QSmartCard *create(PinDialogFactory &pdf);
    ~QSmartCard() override;
    TokenData data() const;
    QSslKey key() const;
    virtual ErrorType login() = 0;
    virtual void logout() = 0;
    virtual QByteArray sign(int type, const QByteArray &dgst) = 0;

public slots:
    virtual void selectCard(const QString &card) = 0;

signals:
    void dataChanged();

protected:
    QSmartCard(PinDialogFactory &pdf, QObject *parent = nullptr);
    class Private;
    Private *d;
};
