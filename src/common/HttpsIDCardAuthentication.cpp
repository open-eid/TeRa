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

#include "HttpsIDCardAuthentication.h"

#include <QSslKey>

namespace ria_tera {

HttpsIDCardAuthentication::HttpsIDCardAuthentication() {}

HttpsIDCardAuthentication::~HttpsIDCardAuthentication() {}

bool HttpsIDCardAuthentication::useIDAuth(QString& url) {
    static const QString ID_CARD_AUTH_PREFIX("#IDCard-AUTH#");

    bool useIDCardAuthentication = false;
    if ((url == "https://puhver.ria.ee/tsa") || (url == "https://puhvertest.ria.ee/tsa")) {
        useIDCardAuthentication = true;
    }
    else if (url.startsWith(ID_CARD_AUTH_PREFIX)) {
        useIDCardAuthentication = true;
        url = url.mid(ID_CARD_AUTH_PREFIX.length()).trimmed();
    }

    return useIDCardAuthentication;
}

void HttpsIDCardAuthentication::setAuthCert(QSslCertificate const& cert, QSslKey const& key) {
    m_authSert = cert;
    m_key = key;
}

void HttpsIDCardAuthentication::addTrustedCerts(QList<QSslCertificate> const& certs) {
    trusted = certs;
}

bool HttpsIDCardAuthentication::isTrusted(QSslCertificate const& request) {
    return trusted.contains(request);
}

void HttpsIDCardAuthentication::configureRequest(QNetworkRequest& request) {
    QSslCertificate cert = m_authSert;

    QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
    QList<QSslCertificate> trusted;
    ssl.setCaCertificates(QList<QSslCertificate>());
    ssl.setProtocol(QSsl::TlsV1_0);
    if (!m_key.isNull())
    {
        ssl.setPrivateKey(m_key);
        ssl.setLocalCertificate(cert);
    }
    request.setSslConfiguration(ssl);
}

}
