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

#include "QSmartCard.h"

#include "TokenData.h"
#ifdef Q_OS_WIN
#include "QCNG.h"
#else
#include "QPKCS11.h"
#endif

#include <openssl/rsa.h>
#include <openssl/ecdsa.h>

class QSmartCard::Private
{
public:
    Private(PinDialogFactory &_pdf): pdf(_pdf) {}
    static int rsa_sign(int type, const unsigned char *m, unsigned int m_len,
        unsigned char *sigret, unsigned int *siglen, const RSA *rsa);
    static ECDSA_SIG* ecdsa_do_sign(const unsigned char *dgst, int dgst_len,
        const BIGNUM *inv, const BIGNUM *rp, EC_KEY *eckey);

#if OPENSSL_VERSION_NUMBER < 0x10010000L || defined(LIBRESSL_VERSION_NUMBER)
    RSA_METHOD		rsamethod = *RSA_get_default_method();
    ECDSA_METHOD	*ecmethod = ECDSA_METHOD_new(nullptr);
#else
    RSA_METHOD		*rsamethod = RSA_meth_dup(RSA_get_default_method());
    EC_KEY_METHOD	*ecmethod = EC_KEY_METHOD_new(nullptr);
#endif

    TokenData selected;
    PinDialogFactory &pdf;
};

#ifdef Q_OS_WIN

class WinCard: public QSmartCard
{
    Q_OBJECT
public:
    WinCard(PinDialogFactory &pdf);
    ~WinCard();

    ErrorType login() override;
    void logout() override;
    void run() override;
    QByteArray sign(int type, const QByteArray &dgst) override;

public slots:
    void selectCard(const QString &card) override;

private:
    QCNG win;
    QCNG::Certs cache;
    mutable bool isRunning = false;
};

#else

class PKCS11Card: public QSmartCard
{
    Q_OBJECT
public:
    PKCS11Card(PinDialogFactory &pdf);
    ~PKCS11Card();
    ErrorType login() override;
    void logout() override;
    void run() override;
    QByteArray sign(int type, const QByteArray &dgst) override;

public slots:
    void selectCard(const QString &card) override;

private:
    QPKCS11Stack stack;
    QList<TokenData> cache;
    mutable bool isRunning = false;
};

#endif
