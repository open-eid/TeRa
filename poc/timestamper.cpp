/*
 * timestamper.cpp
 *
 *  Created on: Oct 26, 2016
 */
// TODO ll
#include "timestamper.h"

#include <iostream>

#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QNetworkReply>
#include <QThreadPool>

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

    QString fileMimetypeName = "mimetype";
    QByteArray fileMimetypeContent = "application/vnd.etsi.asic-s+zip";
    if (!addFile(fileMimetypeName, fileMimetypeContent)) {
        std::cout << "could not add '" << fileMimetypeName.toUtf8().constData() << "' to ddoc" << std::endl; // TODO
        return false;
    }

    insertInputFile(infile); // TODO

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

    error = zip_close(zip);
    if (0 != error) {
        std::cout << "could not finalize '" << outpath.toUtf8().constData() << "'" << std::endl;
        return false;
    }

    return true;
}

bool TimeStamperData_impl::insertInputFile(QString const& path) {
    // https://nih.at/libzip/zip_source_function.html
    zip_source *source = zip_source_file(zip, path.toUtf8().constData(), 0, -1); // TODO
    QFileInfo fileinfo(path);
    int index = (int)zip_file_add(zip, fileinfo.fileName().toUtf8().constData(), source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
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


TimeStamper::TimeStamper() :
        data(new TimeStamperData_impl())
{
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

QByteArray TimeStamper::getTimestampRequest4Sha256(QByteArray& sha256) { // TODO redesign
    return create_timestamp_request(sha256);
}

void TimeStamper::sendTSRequest(QByteArray const& timestampRequest, bool test)
{
    BOOST_LOG_TRIVIAL(info) << "Connecting to time-server: " << timeserverUrl.toUtf8().constData();
    BOOST_LOG_TRIVIAL(trace) << "Request (in Hex):\n" << timestampRequest.toHex().constData();

    QUrl url(timeserverUrl);
    request.setUrl(url);
    request.setRawHeader(QByteArray("Content-Type"), QByteArray("application/timestamp-query"));
    QNetworkReply* r = nam.post(request, timestampRequest);

    if (test) {
        testReplies.insert(r);
    }
}

void TimeStamper::tsReplyFinished(QNetworkReply *reply) {
    bool testRequest = testReplies.contains(reply);
    if (testRequest) {
        testReplies.remove(reply);
    }

    reply->deleteLater();
    if (QNetworkReply::NoError != reply->error()) {
        QString error;
        error.push_back("Time-stamping request failed: ");
        error.push_back(reply->errorString());
        notifyClientOnTimestampingFinished(testRequest, false, error);
        return;
    }

    QByteArray timeserverResponse = reply->readAll();
    BOOST_LOG_TRIVIAL(trace) << "Time-server response (in Hex):\n" << timeserverResponse.toHex().constData();

// TODO verify response against certificate

    QByteArray timestamp = timeserverResponse;

    if (!extract_timestamp_from_ts_response(timeserverResponse, timestamp)) {
        notifyClientOnTimestampingFinished(testRequest, false, "Time-server's response did not contain timestamp.");
        return;
    }

    BOOST_LOG_TRIVIAL(trace) << "Time-stamp (in Hex):\n" << timestamp.toHex().constData();

    if (testRequest) {
        notifyClientOnTimestampingFinished(testRequest, true, "");
        return;
    }

    QFile file(inputFilePath);
    QString fileName = file.fileName();

    BOOST_LOG_TRIVIAL(trace) << "Writing output file: " << outputFilePath.toUtf8().constData();
    //CreateAsicsJob* crawlJob = new CreateAsicsJob(++jobId, testRequest, outputFilePath, inputFilePath, timestamp);
    //QThreadPool::globalInstance()->start(crawlJob);
    if (!data->createAsicsContainer(outputFilePath, inputFilePath, timestamp)) {
        QString error;
        error.push_back("Couldn't create output file '");
        error.push_back(outputFilePath.toUtf8().constData());
        error.push_back("'");
        notifyClientOnTimestampingFinished(testRequest, false, error);
        return;
    } else {
        notifyClientOnTimestampingFinished(testRequest, true, "");
    }
}

CreateAsicsJob::CreateAsicsJob() {
    ;
}

void CreateAsicsJob::run() {
    ;
}

void TimeStamper::notifyClientOnTimestampingFinished(bool test, bool success, QString errString, QByteArray resp) {
    // TODO bad design
    if (test) {
        emit timestampingTestFinished(success, resp, errString);
    } else {
        emit timestampingFinished(success, errString);
    }
}

void TimeStamper::startTimestamping(QString const& tsUrl, QString const& infile, QString const& outfile) {
    QString errorMsg;
    timeserverUrl = tsUrl;
    inputFilePath = infile;
    outputFilePath = outfile;

    QByteArray request;
    bool res = getTimestampRequest(request, errorMsg);
    if (!res) {
        emit timestampingFinished(false, errorMsg);
    } else {
        sendTSRequest(request);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////

OutputNameGenerator::OutputNameGenerator(QString const& inExt, QString const& outExt)
    : inExtension(inExt), outExtension(outExt) {
    if (!inExtension.startsWith(".")) inExtension = "." + inExtension;
    if (!outExtension.startsWith(".")) outExtension = "." + outExtension;
}

QString OutputNameGenerator::getOutFile(QString const& filePath) {
    if (fixedConversion.contains(filePath)) {
        return fixedConversion[filePath];
    }

    QFileInfo fileInfo(filePath);
    QString name = fileInfo.fileName();

    if (name.endsWith(inExtension)) name = name.left(name.size() - inExtension.size());

    QString res;
    int nr = 0;
    do {
        QString newName;
        if (nr > 0) {
            newName = name + "(" + QString::number(nr) + ")" + outExtension;
        } else {
            newName = name + outExtension;
        }
        QFileInfo outFileInfo(QDir(fileInfo.path()), newName);
        if (!outFileInfo.exists()) {
            res = outFileInfo.absoluteFilePath();
        }
        ++nr;
    } while (res.isEmpty());
    return res;
}

void OutputNameGenerator::setFixedOutFile(QString const& in_file, QString const& file_out) {
    fixedConversion[in_file] = file_out;
}


BatchStamper::BatchStamper(StampingMonitorCallback& mon, OutputNameGenerator& ng, bool end_on_first_fail) :
    monitor(mon), namegen(ng), instaFail(end_on_first_fail), pos(-1)
{
    QObject::connect(this, SIGNAL(triggerNext()),
                     this, SLOT(processNext()));
    QObject::connect(&ts, SIGNAL(timestampingFinished(bool,QString)),
                     this, SLOT(timestampFinished(bool,QString)));
}

void BatchStamper::startTimestamping(QString const& tsUrl, QStringList const& inputFiles) {
    pos = -1;
    timeServerUrl = tsUrl;
    input = inputFiles;
    emit triggerNext();
}

TimeStamper& BatchStamper::getTimestamper() {
    return ts;
}

void BatchStamper::processNext() {
    if ((pos+1) >= input.size()) {
        pos = input.size();
        emit timestampingFinished(true, "");
        return;
    }
    ++pos;
    curIn = input[pos];
    curOut = namegen.getOutFile(curIn);
    if (!monitor.processingFile(curIn, curOut, pos, input.size())) {
        emit timestampingFinished(false, tr("Operation cancelled by user...")); // TODO const text see main_window.cpp
        return;
    }
    ts.startTimestamping(timeServerUrl, curIn, curOut);
}

void BatchStamper::timestampFinished(bool success, QString errString) {
    if (!monitor.processingFileDone(curIn, curOut, pos, input.size(), success, errString)) {
        emit timestampingFinished(false, tr("Operation cancelled by user...")); // TODO const text see main_window.cpp
        return;
    }
    if (!success && instaFail) {
        timestampingFinished(success, errString);
    } else {
        emit triggerNext();
    }
}

}
