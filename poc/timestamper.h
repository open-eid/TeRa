/*
 * timestamper.h
 *
 *  Created on: Oct 26, 2016
 */

#ifndef TIMESTAMPER_H_
#define TIMESTAMPER_H_

#include <QObject>
#include <QByteArray>
#include <QScopedPointer>
#include <QString>

#include <QNetworkAccessManager>
#include <QNetworkRequest>

namespace ria_tera {

class TimeStamperData_impl;

class TimeStamper: public QObject {
    Q_OBJECT
public:
    TimeStamper(QString const& infile, QString const& tsUrl, QString const& outfile);

    void startTimestamping();
    bool getTimestampRequest(QByteArray& tsrequest, QString& error);
    void sendTSRequest(QByteArray const& timestampRequest);
    bool createAsicsContainer(QByteArray const& tsresponse);
public slots:
    void tsReplyFinished(QNetworkReply *reply);

    void exitOnFinished(bool success, QString errString);
signals:
    void timestampingFinished(bool success, QString errString);
private:
    QString inputFilePath;
    QString timeserverUrl;
    QString outputFilePath;

    QNetworkAccessManager nam;
    QNetworkRequest request;

    TimeStamperData_impl* data;
};

}

#endif /* TIMESTAMPER_H_ */
