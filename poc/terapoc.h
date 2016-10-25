#ifndef __TeraPOC_H__
#define __TeraPOC_H__

#include <QtConcurrent>
#include <QRunnable>
#include <QThread>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QMutex>
#include <QWaitCondition>
#include <QByteArray>

#include <zip.h>

class TeraPOC : public QObject
{
    Q_OBJECT
public:
    TeraPOC(QString const& sourceFileName, QString const& timeServerUrl, QString const& ddocFileName);
    bool createDDoc();
public slots:
    void tsReplyFinished(QNetworkReply *reply);
private:
    bool readInputFile(QByteArray& ba);
    bool getTimestamp(QByteArray const& data, QByteArray& timeserverResponse);
    bool createZip(QString const& fileName, QByteArray const& fileContent, QByteArray const& timeserverResponse);
    bool addFile(zip_t * zip, QString const& name, QByteArray const& data);
    
    QString inputPath;
    QString tsUrl;
    QString zipPath;
    QNetworkAccessManager nam;
    QByteArray data;
};

#endif
