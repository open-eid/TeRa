/*
 * timestamper.h
 *
 *  Created on: Oct 26, 2016
 */
// TODO ll
#ifndef TIMESTAMPER_H_
#define TIMESTAMPER_H_

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QRunnable>
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
    bool fillTmpAsicsContainer(zip* zip, QString& errorStr);
    bool insertInputFile(zip* zip, QString const& path, QString& errorStr);
    bool addFile(zip* zip, QString const& name, QByteArray const& data, QString& errorStr);
    qint64 jobId;
    QString outpath;
    QString infile;
    QByteArray timestamp;
};

class TimeStamper : public QObject {
    Q_OBJECT
public:
    TimeStamper();

    void startTimestamping(QString const& tsUrl, QString const& infile, QString const& outfile);
    bool getTimestampRequest(QByteArray& tsrequest, QString& error);
    QByteArray getTimestampRequest4Sha256(QByteArray& sha256); // TODO redesign
    void sendTSRequest(QByteArray const& timestampRequest, bool test = false); // TODO redesign
public slots:
    void tsReplyFinished(QNetworkReply *reply);
    void createAsicsContainerFinished(qint64 jobId, bool, QString err);
signals:
    void timestampingFinished(bool success, QString errString);
    void timestampingTestFinished(bool success, QByteArray resp, QString errString);
    void signalAsicsContainerFinished(bool);
private:
    void notifyClientOnTimestampingFinished(bool test, bool success, QString errString, QByteArray resp = QByteArray());

    qint64 jobId;
    QString inputFilePath;
public:
    QString timeserverUrl;
private:
    QString outputFilePath;

    QNetworkAccessManager nam;
    QNetworkRequest request;

    QSet<QNetworkReply*> testReplies;
};

class OutputNameGenerator {
public:
    OutputNameGenerator(QString const& inExt, QString const& outExt);
    QString getOutFile(QString const& filePath);
    void setFixedOutFile(QString const& in_file, QString const& file_out);
private:
    QString inExtension;
    QString outExtension;
    QMap<QString, QString> fixedConversion;
};

class BatchStamper : public QObject {
    Q_OBJECT
public:
    BatchStamper(StampingMonitorCallback& mon, OutputNameGenerator& ng, bool end_on_first_fail);
    void startTimestamping(QString const& tsUrl, QStringList const& inputFiles);
    TimeStamper& getTimestamper();
signals:
    void triggerNext();
    void timestampingFinished(bool success, QString errString);
private slots:
    void processNext();
    void timestampFinished(bool success, QString errString);
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

#endif /* TIMESTAMPER_H_ */
