/*
 * timestamper.cpp
 *
 *  Created on: Oct 26, 2016
 */

#include "timestamper.h"

#include <iostream>

#include <QFileInfo>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QNetworkReply>

#include <zip.h>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>

#include "openssl_utils.h"

namespace ria_tera {

// TODO delete
class TimeStamperData_impl {
public:
    bool createAsicsContainer(QString const& outpath, QString const& infile, QByteArray const& timestamp);
    bool insertInputFile(QString const& path);
    bool addFile(QString const& name, QByteArray const& data);

    zip_t* zip; //TODO
};

bool TimeStamperData_impl::createAsicsContainer(QString const& outpath, QString const& infile, QByteArray const& timestamp) {

    QFileInfo fileinfo(infile);
    QString fileName = fileinfo.fileName();

    int error = 0;

    // https://nih.at/libzip/zip_open.html
    zip = zip_open(outpath.toUtf8().constData(), ZIP_CREATE | ZIP_TRUNCATE, &error); // TODO utf8
    if (error || NULL == zip) {
        std::cout << "could not open or create archive" << std::endl; // TODO
        return false;
    }

    insertInputFile(infile); // TODO

    QString fileMimetypeName = "mimetype";
    QByteArray fileMimetypeContent = "application/vnd.etsi.asic-s+zip";
    if (!addFile(fileMimetypeName, fileMimetypeContent)) {
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
    manifestContent.append("<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.2\">\n");
    manifestContent.append("  <manifest:file-entry manifest:full-path=\"/\" manifest:media-type=\"application/vnd.etsi.asic-e+zip\"/>\n");
    manifestContent.append("  <manifest:file-entry manifest:full-path=\"");
    manifestContent.append(fileName.toUtf8()); // TODO
    manifestContent.append("\" manifest:media-type=\"application/octet-stream\"/>\n");
    manifestContent.append("</manifest:manifest>\n");
    if (!addFile(manifestFileName, manifestContent)) {
        std::cout << "could not add '" << manifestFileName.toUtf8().constData() << "' to ddoc" << std::endl;
        return false;
    }

    QString responseFileName = "META-INF/timestamp.tst"; // TODO
    if (!addFile(responseFileName, timestamp)) {
        std::cout << "could not add '" << responseFileName.toUtf8().constData() << "' to ddoc" << std::endl;
        return false;
    }

    zip_close(zip);
    return true;
}

bool TimeStamperData_impl::insertInputFile(QString const& path) {
    // https://nih.at/libzip/zip_source_function.html
    zip_source *source = zip_source_file(zip, path.toUtf8().constData(), 0, -1); // TODO
    int index = (int)zip_file_add(zip, path.toUtf8().constData(), source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8); ///////////////////////////////////
    if(index < 0)
    {
        std::cout << "failed to add file to archive. " << zip_strerror(zip) << std::endl;
        return false;
    }
    return true;
}

bool TimeStamperData_impl::addFile(QString const& name, QByteArray const& data) {
    // TODO null
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
    return true;
}


TimeStamper::TimeStamper(QString const& infile, QString const& tsUrl, QString const& outfile) :
        inputFilePath(infile),
        timeserverUrl(tsUrl),
        outputFilePath(outfile),
        data(new TimeStamperData_impl())
{
    QUrl url(timeserverUrl);
    request.setUrl(url);
    request.setRawHeader(QByteArray("Content-Type"), QByteArray("application/timestamp-query"));

    QObject::connect(&nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(tsReplyFinished(QNetworkReply*)), Qt::QueuedConnection);
}

static bool calculateSha256(QString const& filePath, QByteArray& sha256, QString& error) {
    QCryptographicHash hashCalculator(QCryptographicHash::Sha256);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        error.clear();
        error.push_back("Couldn't open file '");
        error.push_back(filePath);
        error.push_back("'");
        return false;
    }

    bool res = hashCalculator.addData(&file);
    if (!res) {
        error.clear();
        error.push_back("Couldn't read file '");
        error.push_back(filePath);
        error.push_back("'");
    }
    file.close();

    sha256 = hashCalculator.result();
    return res;
}

bool TimeStamper::getTimestampRequest(QByteArray& tsrequest, QString& error) {
    QByteArray sha256;
    if (!calculateSha256(inputFilePath, sha256, error)) return false;
    tsrequest = create_timestamp_request(sha256);
    return true;
}

void TimeStamper::sendTSRequest(QByteArray const& timestampRequest)
{
    BOOST_LOG_TRIVIAL(info) << "Connecting to time-server: " << timeserverUrl.toUtf8().constData();
    BOOST_LOG_TRIVIAL(trace) << "Request (in Hex):\n" << timestampRequest.toHex().constData();

    QNetworkReply* r = nam.post(request, timestampRequest);
}

void TimeStamper::tsReplyFinished(QNetworkReply *reply) {
    if (QNetworkReply::NoError != reply->error()) {
        QString error;
        error.push_back("Time-stamping request failed: ");
        error.push_back(reply->errorString());
        emit timestampingFinished(false, error);
        return;
    }

    QByteArray timeserverResponse = reply->readAll();
    BOOST_LOG_TRIVIAL(trace) << "Time-server response (in Hex):\n" << timeserverResponse.toHex().constData();

// TODO verify response against certificate

    QByteArray timestamp = timeserverResponse;

    if (!extract_timestamp_from_ts_response(timeserverResponse, timestamp)) {
        emit timestampingFinished(false, "Time-server's response did not contain timestamp.");
        return;
    }

    BOOST_LOG_TRIVIAL(trace) << "Time-stamp (in Hex):\n" << timestamp.toHex().constData();

    QFile file(inputFilePath);
    QString fileName = file.fileName();

    BOOST_LOG_TRIVIAL(trace) << "Writing output file: " << outputFilePath.toUtf8().constData();
    if (!data->createAsicsContainer(outputFilePath, inputFilePath, timestamp)) {
        QString error;
        error.push_back("Couldn't create output file '");
        error.push_back(outputFilePath.toUtf8().constData());
        error.push_back("'");
        emit timestampingFinished(false, error);
        return;
    } else {
        emit timestampingFinished(true, "");
    }
}

void TimeStamper::exitOnFinished(bool success, QString errString) {
    if (success) {
        BOOST_LOG_TRIVIAL(info) << "Timestamping finished successfully :)";
    } else {
        BOOST_LOG_TRIVIAL(error) << "Timestamping failed: " << errString.toUtf8().constData();
    }
    QCoreApplication::exit(0);
}

void TimeStamper::startTimestamping() {
    QString errorMsg;

    QByteArray request;
    bool res = getTimestampRequest(request, errorMsg);
    if (!res) {
        emit timestampingFinished(false, errorMsg);
    } else {
        sendTSRequest(request);
    }
}

}
