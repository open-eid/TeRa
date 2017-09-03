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

// TODO ll
#ifndef TIMESTAMPER_H_
#define TIMESTAMPER_H_

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QRunnable>
#include <QPointer>
#include <QScopedPointer>
#include <QString>

#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include "utils.h"

struct zip;

namespace ria_tera {

class TeraCreateAsicsJob : public QObject, public QRunnable {
    Q_OBJECT
public:
    TeraCreateAsicsJob(qint64 id, QString const& out, QString const& in, QByteArray const& ts);
signals:
    void finished(qint64 jobId, bool asicsSuccess, QString error);
public:
    void run();
    bool createAsicsContainer(QString& errorStr);
private:
    bool fillTmpAsicsContainer(zip* zip, QByteArray const& mimeCont, QString& errorStr);
    bool insertInputFile(zip* zip, QString const& path, QString& errorStr);
    bool addFile(zip* zip, QString const& name, QByteArray const& data, QString& errorStr);
    qint64 jobId;
    QString outpath;
    QString infile;

    // This byte arrays needs to remain untouched after they are added to zip...
    // see https://nih.at/libzip/zip_source_buffer.html
    QByteArray timestamp;
};


class TimeStamperRequestConfigurationFactory {
public:
    virtual bool isTrusted(QSslCertificate const& request) = 0;
    virtual void configureRequest(QNetworkRequest& request) = 0;
};

class TimeStamper : public QObject {
    Q_OBJECT
public:
    TimeStamper();

    void setTimeserverUrl(QString const& url, TimeStamperRequestConfigurationFactory* configurator = NULL); // TODO xxx
    void startTimestamping(QString const& tsUrl, QString const& infile, QString const& outfile);
    bool getTimestampRequest(QByteArray& tsrequest, QString& error);
    QByteArray getTimestampRequest4Sha256(QByteArray& sha256); // TODO redesign
    void sendTSRequest(QByteArray const& timestampRequest, bool test = false, int retries = -1); // TODO redesign

    enum TS_FINISH_DETAILS : int {OTHER, SSL_HANDSHAKE_ERROR};
public slots:
    void tsReplyFinished(QNetworkReply *reply);
    void createAsicsContainerFinished(qint64 jobId, bool, QString err);
signals:
    void timestampingFinished(bool success, QString errString, int details = TS_FINISH_DETAILS::OTHER);
    void timestampingTestFinished(bool success, QByteArray resp, QString errString);
    void signalAsicsContainerFinished(bool);
private:
    void notifyClientOnTimestampingFinished(bool test, bool success, QString errString, TS_FINISH_DETAILS details = TS_FINISH_DETAILS::OTHER, QByteArray resp = QByteArray());

    qint64 jobId;
    QString inputFilePath;
    QString timeserverUrl;
    QString outputFilePath;

    int retriesLeft = 0;
    void* lastPostPtr = nullptr;
    QByteArray lastRequest;

    TimeStamperRequestConfigurationFactory* sslConf;

    QNetworkAccessManager nam;

    QSet<QNetworkReply*> testReplies;
};

class OutputNameGenerator {
public:
    OutputNameGenerator(QStringList const& inExts, QString const& outExt);
    QString getOutFile(QString const& filePath);
    void setFixedOutFile(QString const& in_file, QString const& file_out);
    void setInExts(QStringList const& inExts);
    void setOutExt(QString const& oe);
private:
    QStringList inExtensions;
    QString outExtension;
    QMap<QString, QString> fixedConversion;
};

class BatchStamper : public QObject {
    Q_OBJECT
public:
    class FinishingDetails {
    public:
        bool success = false;
        bool userCancelled = false;
        QString errString;
        FinishingDetails(bool _success = false, QString _errStr = "internal error") : success(_success), errString(_errStr) {};
        static FinishingDetails error(QString const& errStr) {
            FinishingDetails d(false, errStr);
            return d;
        }
        static FinishingDetails cancelled(QString err = QString()) {
            FinishingDetails d(false, err);
            d.userCancelled = true;
            return d;
        };
    };

    BatchStamper(StampingMonitorCallback& mon, OutputNameGenerator& ng, bool end_on_first_fail);
    void startTimestamping(QString const& tsUrl, QStringList const& inputFiles);
    TimeStamper& getTimestamper();
signals:
    void triggerNext();
    void timestampingFinished(FinishingDetails details);
private slots:
    void processNext();
    void timestampFinished(bool success, QString errString, int details);
private:
    StampingMonitorCallback& monitor;
    OutputNameGenerator& namegen;
    bool instaFail;
    int pos;
    QString curIn;
    QString curOut;
    QStringList input;
    QString timeServerUrl;
    TimeStamper ts;
};

}

Q_DECLARE_METATYPE(ria_tera::BatchStamper::FinishingDetails)

#endif /* TIMESTAMPER_H_ */
