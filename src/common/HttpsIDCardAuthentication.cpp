#include "HttpsIDCardAuthentication.h"

#include <QSslKey>

namespace ria_tera {

HttpsIDCardAuthentication::HttpsIDCardAuthentication() {}

HttpsIDCardAuthentication::~HttpsIDCardAuthentication() {}

bool HttpsIDCardAuthentication::useIDAuth(QString& url) {
    static const QString ID_CARD_AUTH_PREFIX("#IDCard-AUTH#");

    bool useIDCardAuthentication = false;
    if (url == "https://puhver.ria.ee/tsa") {
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
