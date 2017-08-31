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

#include <QObject>
#include <QSslKey>

#include "../../poc/timestamper.h"

namespace ria_tera {

class HttpsIDCardAuthentication : public QObject, public TimeStamperRequestConfigurationFactory {
    Q_OBJECT
public:
    HttpsIDCardAuthentication();
    virtual ~HttpsIDCardAuthentication();

    bool useIDAuth(QString& url); // TODO API
    void setAuthCert(QSslCertificate const& cert, QSslKey const& key);
    void addTrustedCerts(QList<QSslCertificate> const& certs);

    bool isTrusted(QSslCertificate const& request);
    void configureRequest(QNetworkRequest& request);
private:
    QSslCertificate m_authSert;
    QSslKey m_key;
    QList<QSslCertificate> trusted;
};

}
