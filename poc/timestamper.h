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
    TimeStamper(QString const& tsUrl);

    void startTimestamping(QString const& infile, QString const& outfile);
    bool getTimestampRequest(QByteArray& tsrequest, QString& error);
    void sendTSRequest(QByteArray const& timestampRequest);
    bool createAsicsContainer(QByteArray const& tsresponse);
public slots:
    void tsReplyFinished(QNetworkReply *reply);
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

class OutputNameGenerator {
public:
    OutputNameGenerator(QString const& ext) : extension(ext) {};
    QString getOutFile(QString const& filePath);
private:
    QString extension;
};

class BatchStamper : public QObject {
    Q_OBJECT
public:
    BatchStamper(QString const& tsUrl, QStringList const& inputFiles, OutputNameGenerator& ng);
    void startTimestamping();
signals:
    void triggerNext();
    void timestampingFinished(bool success, QString errString);
private slots:
    void processNext();
    void timestampFinished(bool success, QString errString);
private:
    int pos;
    QStringList input;
    TimeStamper ts;
    OutputNameGenerator& namegen;
};

}

#endif /* TIMESTAMPER_H_ */
