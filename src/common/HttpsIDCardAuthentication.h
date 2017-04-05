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
