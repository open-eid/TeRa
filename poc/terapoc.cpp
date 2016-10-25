#include "terapoc.h"

#include <iostream>

#include <QCoreApplication>
#include <QThreadPool>
#include <QFuture>

#include <QMutexLocker>

#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QUrl>

TeraPOC::TeraPOC(QString const& sourceFileName, QString const& timeServerUrl, QString const& ddocFileName)
{
    inputPath = sourceFileName;
    tsUrl = timeServerUrl;
    zipPath = ddocFileName;
}

bool TeraPOC::createDDoc()
{
    if (!readInputFile(data)) {
        std::cout << "Can't read input file '" << inputPath.toUtf8().constData() << "'" << std::endl;
        QCoreApplication::exit(1);
        return false;
    }
    
    QByteArray timeserverResponse;
    if (!getTimestamp(data, timeserverResponse)) {
        std::cout << "Could not get response from Time Server" << std::endl;
        QCoreApplication::exit(1);
        return false;
    }
    
    return true;
}

bool TeraPOC::readInputFile(QByteArray& ba)
{
    QFile file(inputPath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    ba = file.readAll();
    return true;
}

QByteArray random8() {
    // TODO random
    return QByteArray::fromHex("4e8eb1fdc0aa5650");
}

QByteArray TSrequest(QByteArray const& hash, QByteArray const& random) {
    // TODO check lengths
    QByteArray tsReq;
    tsReq.append(QByteArray::fromHex("30430201013031300d060960864801650304020105000420"));
    tsReq.append(hash);
    tsReq.append(QByteArray::fromHex("0208"));
    tsReq.append(random);
    tsReq.append(QByteArray::fromHex("0101ff"));
    return tsReq;
}

void TeraPOC::tsReplyFinished(QNetworkReply *reply) {
    QByteArray timeserverResponse = reply->readAll();
    std::cout << "TS response: " << std::endl;
    std::cout << timeserverResponse.toHex().constData() << std::endl;

    QFile file(inputPath);
    QString fileName = file.fileName();

    if (!createZip(fileName, data, timeserverResponse)) {
        std::cout << "Couldn't create output file '" << zipPath.toUtf8().constData() << "'" << std::endl;
        return;
    } else {
        std::cout << "Processing finished :)" << std::endl;
    }
    QCoreApplication::exit(0);
}


bool TeraPOC::getTimestamp(QByteArray const& data, QByteArray& timeserverResponse)
{
    QCryptographicHash hashCalculator(QCryptographicHash::Sha256);
    hashCalculator.addData(data);
    QByteArray hash = hashCalculator.result();

    QByteArray random = random8();

    // get TS request
    QByteArray tsRequest = TSrequest(hash, random);
    
    QUrl* url = new QUrl(tsUrl);
    QNetworkRequest* request = new QNetworkRequest(*url);
    request->setRawHeader(QByteArray("Content-Type"), QByteArray("application/timestamp-query"));

    std::cout << "Connecting to: " << tsUrl.toUtf8().constData() <<  std::endl;
    std::cout << "Request: " << tsRequest.toHex().constData() << std::endl;
    QObject::connect(&nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(tsReplyFinished(QNetworkReply*)), Qt::QueuedConnection);

    QNetworkReply* r = nam.post(*request, tsRequest);
}

bool TeraPOC::createZip(QString const& fileName, QByteArray const& fileContent, QByteArray const& timeserverResponse)
{
    int error = 0;

    // https://nih.at/libzip/zip_open.html
    zip_t* zip = zip_open(zipPath.toUtf8().constData(), ZIP_CREATE | ZIP_TRUNCATE, &error); // TODO utf8
    if (error || NULL == zip) {
        std::cout << "could not open or create archive" << std::endl;
        return false;
    }

    // https://nih.at/libzip/zip_source_function.html
    if (!addFile(zip, fileName, fileContent)) {
        std::cout << "could not add '" << fileName.toUtf8().constData() << "' to ddoc" << std::endl;
        return false;
    }
    
    QString fileMimetypeName = "mimetype";
    QByteArray fileMimetypeContent = "application/vnd.etsi.asic-s+zip";
    if (!addFile(zip, fileMimetypeName, fileMimetypeContent)) {
        std::cout << "could not add '" << fileMimetypeName.toUtf8().constData() << "' to ddoc" << std::endl;
        return false;
    }

    QString metaDirName = "META-INF";
    if (zip_dir_add(zip, metaDirName.toUtf8().constData(), ZIP_FL_ENC_UTF_8) < 0) {
        std::cout << "could not add dir '" << metaDirName.toUtf8().constData() << "' to ddoc" << std::endl;
        return false;
    }

    QString manifestFileName = "META-INF/manifest.xml";
    QByteArray manifestContent;
    manifestContent.append("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n");
    manifestContent.append("<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\">\n");
    manifestContent.append("  <manifest:file-entry manifest:full-path=\"/\" manifest:media-type=\"application/vnd.etsi.asic-e+zip\"/>\n");
    manifestContent.append("  <manifest:file-entry manifest:full-path=\"");
    manifestContent.append(fileName.toUtf8()); // TODO
    manifestContent.append("\" manifest:media-type=\"application/octet-stream\"/>\n");
    manifestContent.append("</manifest:manifest>\n");
    if (!addFile(zip, manifestFileName, manifestContent)) {
        std::cout << "could not add '" << manifestFileName.toUtf8().constData() << "' to ddoc" << std::endl;
        return false;
    }

    QString responseFileName = "META-INF/response.tsr";
    if (!addFile(zip, responseFileName, timeserverResponse)) {
        std::cout << "could not add '" << responseFileName.toUtf8().constData() << "' to ddoc" << std::endl;
        return false;
    }

    zip_close(zip);
    return true;
}

bool TeraPOC::addFile(zip_t * zip, QString const& name, QByteArray const& data)
{
    zip_source *source = zip_source_buffer(zip, data.constData(), data.length(), 0);
    if(source == NULL)
    {
        std::cout << "failed to create source buffer. " << zip_strerror(zip) << std::endl;
        return false;
    }

    int index = (int)zip_file_add(zip, name.toUtf8().constData(), source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
    if(index < 0)
    {
        std::cout << "failed to add file to archive. " << zip_strerror(zip) << std::endl;
        return false;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc < 3+1) {
        std::cout << "Usage: " << std::endl;
        std::cout << " " << argv[0] << " <f1> <url> <zip>" << std::endl;
        std::cout << std::endl;
        std::cout << "Example:" << std::endl;
        std::cout << " " << argv[0] << " test.txt http://demo.sk.ee/tsa test.zip" << std::endl;
        return 0;
    }

    TeraPOC m(argv[1], argv[2], argv[3]);
    if (m.createDDoc()) {
        return a.exec();
    } else {
        return 1;
    }
}
