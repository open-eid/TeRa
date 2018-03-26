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

#include "cmdline_timestamper_processor.h"

#include <iostream>
#include <string>
#include <sstream>

#include <QMutex>
#include <QWaitCondition>

#include "poc/config.h"
#include "common/SslCertificate.h"
#include "src/libdigidoc/Configuration.h"

#ifdef WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace {

void SetStdinEcho(bool enable = true)
{
#ifdef WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE); 
    DWORD mode;
    GetConsoleMode(hStdin, &mode);

    if( !enable )
        mode &= ~ENABLE_ECHO_INPUT;
    else
        mode |= ENABLE_ECHO_INPUT;

    SetConsoleMode(hStdin, mode );

#else
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if( !enable )
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;

    (void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

}

namespace ria_tera {

class CmdLinePinDialog : public PinDialogInterface {
private:
    PinDialogInterface::PinFlags flags;
    bool pinpadPinDone = false;
    int pinpadPinResult = -1;
    QByteArray pin;

    QMutex mutex;
    QWaitCondition wait;
public:
    CmdLinePinDialog(PinDialogInterface::PinFlags f) : flags(f) {}

    virtual bool execDialog() {
        if (flags & PinpadFlag) {
            std::cout << "Enter PIN1 from pinpad for authentication in time-server...";
            QMutexLocker g(&mutex);
            wait.wait(&mutex);
            return 0 == pinpadPinResult;
        } else {
            std::string _pin1;
            std::cout << "Enter PIN1 for authentication in time-server: ";

            SetStdinEcho(false);
            std::cin >> _pin1;
            SetStdinEcho(true);

            std::cout << std::endl;

            pin = _pin1.c_str();
            return true;
        }
    }

    virtual QByteArray getPin() {
        return pin;
    }

    /// doStartTimer & doFinish are called in that order together with execDialog
    /// after doFinish execDialog should end with false
    virtual void doStartTimer() {
    }

    virtual void doFinish(int result) {
        pinpadPinDone = true;
        pinpadPinResult = result;

        QMutexLocker g(&mutex);
        wait.wakeAll();
    }
};

PinDialogInterface* TeRaMonitor::createPinDialog(PinDialogInterface::PinFlags flags, const QSslCertificate &cert) {
    return new CmdLinePinDialog(flags);
}

TeRaMonitor::TeRaMonitor() {
    QObject::connect(this, &TeRaMonitor::kickstart_signal,
        this, &TeRaMonitor::stepStartProcess, Qt::QueuedConnection); // queued connection needed to ensure a.exec() catches exit
    QObject::connect(this, &TeRaMonitor::signal_stepSelectCard, this, &TeRaMonitor::stepSelectCard);
    QObject::connect(this, &TeRaMonitor::signal_stepAuthenticatePIN1, this, &TeRaMonitor::stepAuthenticatePIN1);
    QObject::connect(this, &TeRaMonitor::signal_stepFindAndStamp, this, &TeRaMonitor::stepFindAndStamp);

    connect(&Configuration::instance(), SIGNAL(finished(bool, const QString&)), this, SLOT(globalConfFinished(bool, const QString&)), Qt::QueuedConnection);
    connect(&Configuration::instance(), SIGNAL(networkError(const QString&)), this, SLOT(globalConfNetworkError(const QString&)));
}

void TeRaMonitor::kickstart(QString const& ts_url, IOParameters const& iop) {
    time_server_url_original = ts_url;
    io_params = iop;

    Configuration::instance().update();
}

static QString const INI_FILE_DEFAULTS(":/tera.ini");
static QString const INI_GROUP("tera");
static QString const INI_GROUP_ = INI_GROUP + "/";
static QString const INI_TRUSTED_CERT = Config::INI_GROUP_ + "time_server.trusted_cert";

void TeRaMonitor::globalConfFinished(bool changed, const QString &error) {
    idCardAuth.addTrustedCerts(config.getTrustedHttpsCerts());
    emit kickstart_signal();
}

void TeRaMonitor::globalConfNetworkError(const QString &error) {
    TERA_LOG(error) << "Please check internet connection. Error when downloading configuration: " << error;
    QCoreApplication::exit(2);
}

void TeRaMonitor::stepStartProcess() {
    time_server_url = time_server_url_original;
    useIDCardAuthentication = idCardAuth.useIDAuth(time_server_url);

    if (useIDCardAuthentication) {
        smartCard.reset(QSmartCard::create(*this));
        connect(smartCard.data(), SIGNAL(dataChanged()), this, SLOT(cardDataChanged()), Qt::QueuedConnection);
        smartCard->start();
        std::cout << "Looking for ID-card..." << std::endl;
        // wait for cardDataChanged to be triggered
    } else {
        emit signal_stepFindAndStamp();
    }
}

void TeRaMonitor::cardDataChanged() {
    smartCardData = smartCard->dataXXX();
    if (ID_AUTH_STATE::WAIT_CARD_LIST == idAuthState) {
        QString documentId = smartCardData.card();
        if (documentId.isNull()) {
            return;
        }

        QStringList cards = smartCardData.cards();

        if (0 == cards.size()) {
            qDebug() << "!!!!!!!!!!!!! No ID-cards inserted";
        } if (cards.size() == 1 || smartCardData.card() == selectedCard) {
            QString fname = smartCardData.cert().subjectInfo("GN").join(" ");
            fname = fname.trimmed();
            QString surname = smartCardData.cert().subjectInfo("SN").join(" ");
            QString userid = smartCardData.cert().subjectInfo("serialNumber").join(" ");

            TERA_LOG(info) << "Authenticating using ID-card";
            TERA_LOG(info) << "Document ID: " << documentId;
            TERA_LOG(info) << "Given names: " << fname;
            TERA_LOG(info) << "Surname: " << surname;
            TERA_LOG(info) << "Personal Code: " << userid;

            idAuthState = ID_AUTH_STATE::WAIT_PIN;
            emit signal_stepAuthenticatePIN1();
        } else if (cards.size() > 1) {
            std::cout << "Multiple ID-cards connected. Please select one" << std::endl;
            for (int i = 0; i < cards.size(); ++i) {
                std::cout << " " << (i+1) << ") " << cards.at(i).toUtf8().constData() << std::endl;
            }
            std::cout << "Please select which one to use: ";

            std::string selection;
            std::cin >> selection;

            std::stringstream is;
            is.str(selection);
            int sel_i;
            is >> sel_i;

            if (!is.fail() && 1 <= sel_i && sel_i <= cards.size()+1) {
                selectedCard = cards.at(sel_i-1);
                smartCard->selectCard(selectedCard);
            } else {
                std::cout << "Illegal selection" << std::endl;
                QCoreApplication::exit(3);
            }
        }
    } else if (ID_AUTH_STATE::WAIT_PIN == idAuthState) {
    }
}

void TeRaMonitor::stepSelectCard() {
    ;
}

void TeRaMonitor::stepAuthenticatePIN1() {
    if (smartCard.isNull()) return; // TODO error
    QSmartCard::ErrorType error = smartCard->login();
    if (QSmartCard::ErrorType::NoError != error) {
        smartCardData = smartCard->dataXXX();
        QString message;

        if (QSmartCard::ErrorType::ValidateError == error) {
            message = QString("Wrong PIN1.");
        } else if (QSmartCard::ErrorType::BlockedError == error) {
            message = "PIN1 is blocked.";
        } else if (QSmartCard::ErrorType::UnknownError == error) {
            message = "Error occurred while verifying PIN.\nPlease check if ID-card is still in the reader.";
            smartCard->logout();
        } else {
            message = "Error: " + QString::number(error); // TODO
            smartCard->logout();
        }
        std::cout << "Error on authentication: " << message.toUtf8().constData() << std::endl;
        QCoreApplication::exit(4);
    } else {
        smartCardData = smartCard->dataXXX();
        idCardAuth.setAuthCert(smartCardData.cert(), smartCard->key()); // TODO API
        emit signal_stepFindAndStamp();
    }
}

void TeRaMonitor::stepFindAndStamp() {
    namegen.reset(new ria_tera::OutputNameGenerator(ria_tera::Config::IN_EXTENSIONS, io_params.out_extension));

    QStringList inFiles;
    if (io_params.in_file.isEmpty()) {
        TERA_COUT("Searching for extensions *.(" << QSTR_TO_CCHAR(io_params.in_extensions.join(", ")) << ").");
        ria_tera::DiskCrawler dc(*this, io_params.in_extensions);
        dc.addExcludeDirs(io_params.excl_dirs);
        dc.addInputDir(io_params.in_dir, io_params.in_dir_recursive);
        inFiles = dc.crawl();
    }
    else {
        //any file is valid for timestamping process here by force
        inFiles.append(io_params.in_file);
        namegen->setFixedOutFile(io_params.in_file, io_params.file_out);
    }

    if (0 == inFiles.size()) {
        TERA_COUT("No *.(" << QSTR_TO_CCHAR(io_params.in_extensions.join(", ")) << ") files selected for timestamping.");
    }
    stamper.reset(new ria_tera::BatchStamper(*this, *namegen, false));

    QObject::connect(stamper.data(), &ria_tera::BatchStamper::timestampingFinished,
        this, &ria_tera::TeRaMonitor::exitOnFinished, Qt::QueuedConnection); // queued connection needed to ensure a.exec() catches exit

    stamper->getTimestamper().setTimeserverUrl(time_server_url, (useIDCardAuthentication ? &idCardAuth : nullptr));
    stamper->startTimestamping(time_server_url, inFiles); // TODO error to XXX when network is down for example
}

void TeRaMonitor::exitOnFinished(ria_tera::BatchStamper::FinishingDetails d) {
    if (d.success && 0 == failedCnt && succeededCnt == foundCnt) {
        TERA_COUT("Timestamping finished successfully :)");
        QCoreApplication::exit(0);
    } else {
        TERA_LOG(error) << "Timestamping finished with some errors :(";
        TERA_LOG(error) << "   Successfully converted: " << succeededCnt;
        TERA_LOG(error) << "   Number of failed coversions: " << failedCnt;
        int skipped = foundCnt - succeededCnt - failedCnt;
        if (0 != skipped) {
            TERA_LOG(error) << "   Number of DDOCs skipped: " << skipped;
        }
        if (!d.success) {
            TERA_LOG(error) << "Error: " << d.errString.toUtf8().constData();
        }
        QCoreApplication::exit(1);
    }
    if (!smartCard.isNull()) {
        smartCard->logout();
    }
}

}
