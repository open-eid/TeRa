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

#include "logging.h"
#include "openssl_utils.h"

namespace ria_tera {

    TeraCreateAsicsJob::TeraCreateAsicsJob(qint64 id, QString const& out, QString const& in, QByteArray const& ts)
    : jobId(id), outpath(out), infile(in), timestamp(ts)
{
}

void TeraCreateAsicsJob::run() {
    QString errorStr;
    bool res = createAsicsContainer(errorStr);
    emit finished(jobId, res, errorStr);
}

bool TeraCreateAsicsJob::createAsicsContainer(QString& errorStr) {
    int error = 0;

    // open tmp zip file
    // https://nih.at/libzip/zip_open.html
    zip_t* zip = zip_open(outpath.toUtf8().constData(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (error || NULL == zip) {
        errorStr = QString("Could not create/open archive %1").arg(outpath);
        return false;
    }

    // create zip file
    QByteArray fileMimetypeContent = "application/vnd.etsi.asic-s+zip";

    QByteArray manifestContent;
    manifestContent.append("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n");
    manifestContent.append("<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.2\">\n");
    manifestContent.append("  <manifest:file-entry manifest:full-path=\"/\" manifest:media-type=\"application/vnd.etsi.asic-e+zip\"/>\n");
    manifestContent.append("  <manifest:file-entry manifest:full-path=\"");
    manifestContent.append(QFileInfo(infile).fileName().toUtf8());
    manifestContent.append("\" manifest:media-type=\"application/octet-stream\"/>\n");
    manifestContent.append("</manifest:manifest>\n");

    bool res = fillTmpAsicsContainer(zip, fileMimetypeContent, manifestContent, errorStr);

    // close zip file
    if (res) {
        error = zip_close(zip);
        if (0 != error) {
            errorStr = QString("Could not finalize '%1'").arg(outpath);
            zip_discard(zip);
            return false;
        }
    } else {
        errorStr = QString("Error while creating '%1': %2").arg(outpath, errorStr);
        zip_discard(zip);
        return false;
    }

    return true;
}

bool TeraCreateAsicsJob::fillTmpAsicsContainer(zip* zip, QByteArray const& mimeCont, QByteArray const& manifestCont, QString& errorStr) {
    int error = 0;

    if (!addFile(zip, "mimetype", mimeCont, errorStr)) return false;

    if (!insertInputFile(zip, infile, errorStr)) return false;

    QString metaDirName = "META-INF";
    if (zip_dir_add(zip, metaDirName.toUtf8().constData(), ZIP_FL_ENC_UTF_8) < 0) {
        errorStr = QString("could not add dir '%1' to ddoc").arg(metaDirName);
        return false;
    }

    if (!addFile(zip, "META-INF/manifest.xml", manifestCont, errorStr)) return false;

    if (!addFile(zip, "META-INF/timestamp.tst", timestamp, errorStr)) return false;

    return true;
}

bool TeraCreateAsicsJob::insertInputFile(zip_t* zip, QString const& path, QString& errorStr) {
    // https://nih.at/libzip/zip_source_function.html
    zip_source *source = zip_source_file(zip, path.toUtf8().constData(), 0, -1); // TODO proper cancelling
    if (source == NULL)
    {
        errorStr = QString("could not add '%1' to ddoc - failed to create source file. %2").arg(path, zip_strerror(zip));
        return false;
    }

    QFileInfo fileinfo(path);
    int index = (int)zip_file_add(zip, fileinfo.fileName().toUtf8().constData(), source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
// TODO    zip_source_free(source);
    if(index < 0)
    {
        errorStr = QString("failed to add file '%1' to archive - %2").arg(path, zip_strerror(zip));
        return false;
    }
    return true;
}

bool TeraCreateAsicsJob::addFile(zip_t* zip, QString const& name, QByteArray const& data, QString& errorStr) {
    zip_source *source = zip_source_buffer(zip, data.constData(), data.length(), 0);
    if(source == NULL)
    {
        errorStr = QString("could not add '%1' to ddoc - failed to create source buffer. %2").arg(name, zip_strerror(zip));
        return false;
    }

    int index = (int)zip_file_add(zip, name.toUtf8().constData(), source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
// TODO    zip_source_free(source);
    if(index < 0)
    {
        errorStr = QString("could not add '%1' to ddoc - failed to add file to archive. %2").arg(name, zip_strerror(zip));
        return false;
    }
    return true;
}


TimeStamper::TimeStamper() : jobId(0)
{
    QObject::connect(&nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(tsReplyFinished(QNetworkReply*)));
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
    TERA_LOG(debug) << "Connecting to time-server: " << timeserverUrl.toUtf8().constData();
    TERA_LOG(trace) << "Request (in Hex):\n" << timestampRequest.toHex().constData();

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
    TERA_LOG(trace) << "Time-server response (in Hex):\n" << timeserverResponse.toHex().constData();

// TODO verify response against certificate

    QByteArray timestamp = timeserverResponse;

    if (!extract_timestamp_from_ts_response(timeserverResponse, timestamp)) {
        notifyClientOnTimestampingFinished(testRequest, false, "Time-server's response did not contain timestamp.");
        return;
    }

    TERA_LOG(trace) << "Time-stamp (in Hex):\n" << timestamp.toHex().constData();

    if (testRequest) {
        notifyClientOnTimestampingFinished(testRequest, true, "");
        return;
    }

    QFile file(inputFilePath);
    QString fileName = file.fileName();

    TERA_LOG(trace) << "Writing output file: " << outputFilePath.toUtf8().constData();
    TeraCreateAsicsJob* createAsicsJob = new TeraCreateAsicsJob(++jobId, outputFilePath, inputFilePath, timestamp);
    QObject::connect(createAsicsJob, &TeraCreateAsicsJob::finished, this, &TimeStamper::createAsicsContainerFinished);
    QThreadPool::globalInstance()->start(createAsicsJob);
}

void TimeStamper::createAsicsContainerFinished(qint64 doneJobId, bool asicsSuccess, QString err) {
    if (jobId != doneJobId) return;

    QString error;
    if (!asicsSuccess) { // TODO ... error is not necessary, err should contain everything
        error.push_back("Couldn't create output file '");
        error.push_back(outputFilePath.toUtf8().constData());
        error.push_back("': ");
        error.push_back(err);
    }
    notifyClientOnTimestampingFinished(false, asicsSuccess, error);
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
