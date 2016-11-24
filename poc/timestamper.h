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
#include <QScopedPointer>
#include <QString>

#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include "utils.h"

namespace ria_tera {

class TimeStamperData_impl;

class TimeStamper: public QObject {
    Q_OBJECT
public:
    TimeStamper();

    void startTimestamping(QString const& tsUrl, QString const& infile, QString const& outfile);
    bool getTimestampRequest(QByteArray& tsrequest, QString& error);
    QByteArray getTimestampRequest4Sha256(QByteArray& sha256); // TODO redesign
    void sendTSRequest(QByteArray const& timestampRequest, bool test = false); // TODO redesign
    bool createAsicsContainer(QByteArray const& tsresponse);
public slots:
    void tsReplyFinished(QNetworkReply *reply);
signals:
    void timestampingFinished(bool success, QString errString);
    void timestampingTestFinished(bool success, QByteArray resp, QString errString);
private:
    void notifyClientOnTimestampingFinished(bool test, bool success, QString errString, QByteArray resp = QByteArray());

    QString inputFilePath;
public:
    QString timeserverUrl;
private:
    QString outputFilePath;

    QNetworkAccessManager nam;
    QNetworkRequest request;

    QSet<QNetworkReply*> testReplies;

    TimeStamperData_impl* data;
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
    BatchStamper(ProcessingMonitorCallback& mon, OutputNameGenerator& ng);
    void startTimestamping(QString const& tsUrl, QStringList const& inputFiles);
    TimeStamper& getTimestamper();
signals:
    void triggerNext();
    void timestampingFinished(bool success, QString errString);
private slots:
    void processNext();
    void timestampFinished(bool success, QString errString);
private:
    ProcessingMonitorCallback& monitor;
    OutputNameGenerator& namegen;
    int pos;
    QStringList input;
    QString timeServerUrl;
    TimeStamper ts;
};

}

#endif /* TIMESTAMPER_H_ */
