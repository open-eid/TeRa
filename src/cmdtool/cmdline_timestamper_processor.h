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

#include <QCoreApplication>

#include "poc/logging.h"
#include "poc/config.h"
#include "poc/disk_crawler.h"
#include "poc/timestamper.h"

#include "common/PinDialog.h"
#include "common/QSmartCard.h"
#include "src/common/HttpsIDCardAuthentication.h"

namespace ria_tera {

class TeRaMonitor : public QObject, public ria_tera::ProcessingMonitorCallback, public PinDialogFactory {
    Q_OBJECT
public:
    class IOParameters {
    public:
        QString out_extension;
        QString in_file;
        QStringList excl_dirs;
        QString in_dir;
        bool in_dir_recursive = false;
        QString file_out;
    };
private:
    enum ID_AUTH_STATE {WAIT_CARD_LIST, WAIT_PIN};
    Config config;

    QString time_server_url_original;
    QString time_server_url;
    bool useIDCardAuthentication = false;
    IOParameters io_params;

    ID_AUTH_STATE idAuthState = ID_AUTH_STATE::WAIT_CARD_LIST;
    QSharedPointer<QSmartCard> smartCard;
    QString selectedCard;
    QSmartCardData smartCardData;
    HttpsIDCardAuthentication idCardAuth;

    QScopedPointer<ria_tera::OutputNameGenerator> namegen;
    QScopedPointer<ria_tera::BatchStamper> stamper;
public:
    virtual PinDialogInterface* createPinDialog(PinDialogInterface::PinFlags flags, const QSslCertificate &cert);

    void kickstart(QString const& ts_url, IOParameters const& iop);
signals:
    void kickstart_signal();
    void signal_stepSelectCard();
    void signal_stepAuthenticatePIN1();
    void signal_stepFindAndStamp();
public slots:
    void stepStartProcess();
    void stepSelectCard();
    void stepAuthenticatePIN1();
    void stepFindAndStamp();

    void globalConfFinished(bool changed, const QString &error);
    void globalConfNetworkError(const QString &error);

    void cardDataChanged();

    void exitOnFinished(ria_tera::BatchStamper::FinishingDetails d);
public:
    TeRaMonitor();
    bool processingPath(QString const& path, double progress_percent) {
        TERA_COUT("Searching " << path.toUtf8().constData());
        return true;
    };
    bool excludingPath(QString const& path) {
        TERA_COUT("   Excluding " << path.toUtf8().constData());
        return true;
    };
    bool foundFile(QString const& path) {
        TERA_COUT("   Found " << path.toUtf8().constData());
        return true;
    };
    bool processingFile(QString const& pathIn, QString const& pathOut, int nr, int totalCnt) {
        TERA_COUT("Timestamping (" << (nr+1) << "/" << totalCnt << ") " << pathIn.toUtf8().constData() <<
                " -> " << pathOut.toUtf8().constData());
        return true;
    };
    bool processingFileDone(QString const& pathIn, QString const& pathOut, int nr, int totalCnt, bool success, QString const& errString) {
        foundCnt = totalCnt;
        if (success) {
            succeededCnt++;
        } else {
            failedCnt++;
            TERA_LOG(error) << "   Error converting " << pathIn.toUtf8().constData() << ": " << errString.toUtf8().constData();
        }
        return true;
    };
private:
    int foundCnt = 0;
    int succeededCnt = 0;
    int failedCnt = 0;
};


}
