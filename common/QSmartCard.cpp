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

#include "QSmartCard_p.h"

#include <QSslKey>

#if OPENSSL_VERSION_NUMBER < 0x10010000L
static int ECDSA_SIG_set0(ECDSA_SIG *sig, BIGNUM *r, BIGNUM *s)
{
    if(!r || !s)
        return 0;
    BN_clear_free(sig->r);
    BN_clear_free(sig->s);
    sig->r = r;
    sig->s = s;
    return 1;
}
#endif



int QSmartCard::Private::rsa_sign(int type, const unsigned char *m, unsigned int m_len,
        unsigned char *sigret, unsigned int *siglen, const RSA *rsa)
{
    QSmartCard *d = (QSmartCard*)RSA_get_app_data(rsa);
    QByteArray result = d->sign(type, QByteArray::fromRawData((const char*)m, int(m_len)));
    if(result.isEmpty())
        return 0;
    *siglen = (unsigned int)result.size();
    memcpy(sigret, result.constData(), size_t(result.size()));
    return 1;
}

ECDSA_SIG* QSmartCard::Private::ecdsa_do_sign(const unsigned char *dgst, int dgst_len,
        const BIGNUM *, const BIGNUM *, EC_KEY *eckey)
{
#if OPENSSL_VERSION_NUMBER < 0x10010000L
    QSmartCard *d = (QSmartCard*)ECDSA_get_ex_data(eckey, 0);
#else
    QSmartCard *d = (QSmartCard*)EC_KEY_get_ex_data(eckey, 0);
#endif
    QByteArray result = d->sign(0, QByteArray::fromRawData((const char*)dgst, int(dgst_len)));
    if(result.isEmpty())
        return nullptr;
    QByteArray r = result.left(result.size()/2);
    QByteArray s = result.right(result.size()/2);
    ECDSA_SIG *sig = ECDSA_SIG_new();
    ECDSA_SIG_set0(sig,
        BN_bin2bn((const unsigned char*)r.data(), int(r.size()), 0),
        BN_bin2bn((const unsigned char*)s.data(), int(s.size()), 0));
    return sig;
}



QSmartCard::QSmartCard(PinDialogFactory &pdf)
    : d(new Private(pdf))
{
#if OPENSSL_VERSION_NUMBER < 0x10010000L || defined(LIBRESSL_VERSION_NUMBER)
    d->rsamethod.name = "QSmartCard";
    d->rsamethod.rsa_sign = Private::rsa_sign;
    ECDSA_METHOD_set_name(d->ecmethod, const_cast<char*>("QSmartCard"));
    ECDSA_METHOD_set_sign(d->ecmethod, Private::ecdsa_do_sign);
    ECDSA_METHOD_set_app_data(d->ecmethod, const_cast<QSmartCard*>(this));
#else
    RSA_meth_set1_name(d->rsamethod, "QSmartCard");
    RSA_meth_set_sign(d->rsamethod, Private::rsa_sign);
    EC_KEY_METHOD_set_sign(d->ecmethod, nullptr, nullptr, Private::ecdsa_do_sign);
#endif
}

QSmartCard::~QSmartCard()
{
#if OPENSSL_VERSION_NUMBER >= 0x10010000L
    RSA_meth_free(d->rsamethod);
    EC_KEY_METHOD_free(d->ecmethod);
#else
    ECDSA_METHOD_free(d->ecmethod);
#endif
    delete d;
}

QSmartCard* QSmartCard::create(PinDialogFactory &pdf)
{
#ifdef Q_OS_WIN
    return new WinCard(pdf);
#else
    return new PKCS11Card(pdf);
#endif
}

QSslKey QSmartCard::key() const
{
    QSslKey key = d->selected.cert().publicKey();
    if(!key.handle())
        return key;
    if (key.algorithm() == QSsl::Ec)
    {
        EC_KEY *ec = (EC_KEY*)key.handle();
#if OPENSSL_VERSION_NUMBER < 0x10010000L
        ECDSA_set_ex_data(ec, 0, const_cast<QSmartCard*>(this));
        ECDSA_set_method(ec, d->ecmethod);
#else
        EC_KEY_set_ex_data(ec, 0, const_cast<QSmartCard*>(this));
        EC_KEY_set_method(ec, d->ecmethod);
#endif
    }
    else
    {
        RSA *rsa = (RSA*)key.handle();
#if OPENSSL_VERSION_NUMBER < 0x10010000L || defined(LIBRESSL_VERSION_NUMBER)
        RSA_set_method(rsa, &d->rsamethod);
        rsa->flags |= RSA_FLAG_SIGN_VER;
#else
        RSA_set_method(rsa, d->rsamethod);
#endif
        RSA_set_app_data(rsa, const_cast<QSmartCard*>(this));
    }
    return key;
}

TokenData QSmartCard::dataXXX() const
{
    return d->selected;
}

#ifdef Q_OS_WIN

WinCard::WinCard(PinDialogFactory &pdf)
    : QSmartCard(pdf)
{
}

WinCard::~WinCard()
{
    isRunning = false;
    wait();
}

WinCard::ErrorType WinCard::login()
{
    return NoError;
}

void WinCard::logout()
{
}

void WinCard::run()
{
    isRunning = true;
    while (isRunning)
    {
        QList<TokenData> tmp = win.tokens();
        for(QList<TokenData>::iterator i = tmp.begin(); i != tmp.end();)
        {
            if(!SslCertificate(i->cert()).enhancedKeyUsage().contains(SslCertificate::ClientAuth))
                i = tmp.erase(i);
            else
                ++i;
        }
        if (tmp != cache)
        {
            cache = tmp;
            if(!tmp.isEmpty())
                selectCard(tmp.front().card());
            Q_EMIT dataChanged();
        }
        sleep(5*1000);
    }
}

void WinCard::selectCard(const QString &card)
{
    for(QList<TokenData>::const_iterator i = cache.cbegin(); i != cache.cend(); ++i)
    {
        if(i->card() == card)
        {
            TokenData token;
            token.setCard(i->card());
            token.setCert(i->cert());
            d->selected = token;
            win.selectCert(i->cert());
        }
    }
}

QByteArray WinCard::sign(int type, const QByteArray &dgst)
{
    return win.sign(type, dgst);
}

#else

PKCS11Card::PKCS11Card(PinDialogFactory &pdf)
    : QSmartCard(pdf)
{}

PKCS11Card::~PKCS11Card()
{
    isRunning = false;
    wait();
}

PKCS11Card::ErrorType PKCS11Card::login()
{
    switch (stack.login(d->selected, d->pdf))
    {
    case QPKCS11::PinOK: return NoError;
    case QPKCS11::PinCanceled: return CancelError;
    case QPKCS11::PinIncorrect: return ValidateError;
    case QPKCS11::PinLocked: return BlockedError;
    default: return UnknownError;
    }
}

void PKCS11Card::logout()
{
    stack.logout();
}

void PKCS11Card::run()
{
    isRunning = true;
    stack.load();
    while (isRunning)
    {
        QList<TokenData> tmp = stack.tokens();
        if (tmp != cache)
        {
            cache = tmp;
            if(!tmp.isEmpty())
                d->selected = tmp[0];
            Q_EMIT dataChanged();
        }
        sleep(5*1000);
    }
}

void PKCS11Card::selectCard(const QString &card)
{
    d->selected.clear();
    for(const TokenData &token: cache)
        if(token.card() == card)
            d->selected = token;
}

QByteArray PKCS11Card::sign(int type, const QByteArray &dgst)
{
    return stack.sign(type, dgst);
}

#endif
