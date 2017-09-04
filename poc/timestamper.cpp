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
#include "timestamper.h"

#include <iostream>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QNetworkReply>
#include <QThreadPool>
#include <QTimer>

#include <zip.h>

#ifdef LIBZIP_VERSION_MAJOR
#if LIBZIP_VERSION_MAJOR == 0 && LIBZIP_VERSION_MINOR <= 10
    typedef struct zip zip_t;
    typedef struct zip_source zip_source_t;

    #define ZIP_TRUNCATE 0

    #define OLD_LIBZIP_NO_ZIP_DISCARD

    #define ZIP_FL_ENC_UTF_8 0
    ZIP_EXTERN zip_int64_t zip_dir_add(zip_t *za, const char *name, int) {return zip_add_dir(za, name);};

    #define ZIP_FL_OVERWRITE 0
    ZIP_EXTERN zip_int64_t zip_file_add(zip_t *za, const char *name, zip_source_t *s, int) {return zip_add(za, name,s);};
#endif
#endif

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
    zip_t* zip = zip_open(outpath.toUtf8().constData(), ZIP_CREATE | ZIP_EXCL, &error);
    if (error || NULL == zip) {
        errorStr = QString("Could not create/open archive %1").arg(outpath);
        return false;
    }

    // create zip file
    QByteArray fileMimetypeContent = "application/vnd.etsi.asic-s+zip";

    bool res = fillTmpAsicsContainer(zip, fileMimetypeContent, errorStr);

    // close zip file
    if (res) {
        error = zip_close(zip);
        if (0 != error) {
            errorStr = QString("Could not finalize '%1'").arg(outpath);
#ifndef OLD_LIBZIP_NO_ZIP_DISCARD
            zip_discard(zip);
#endif
            return false;
        }
    } else {
        errorStr = QString("Error while creating '%1': %2").arg(outpath, errorStr);
#ifndef OLD_LIBZIP_NO_ZIP_DISCARD
        zip_discard(zip);
#else
        zip_unchange_archive(zip);
        zip_close(zip); // TODO what to do with failed zip-file
#endif
        return false;
    }

    return true;
}

bool TeraCreateAsicsJob::fillTmpAsicsContainer(zip* zip, QByteArray const& mimeCont, QString& errorStr) {
    int error = 0;

    if (!addFile(zip, "mimetype", mimeCont, errorStr)) return false;

    if (!insertInputFile(zip, infile, errorStr)) return false;

    QString metaDirName = "META-INF";
    if (zip_dir_add(zip, metaDirName.toUtf8().constData(), ZIP_FL_ENC_UTF_8) < 0) {
        errorStr = QString("could not add dir '%1' to ddoc").arg(metaDirName);
        return false;
    }

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


TimeStamper::TimeStamper() : jobId(0), sslConf(nullptr)
{
    QObject::connect(&nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(tsReplyFinished(QNetworkReply*)));
    QObject::connect(&nam, &QNetworkAccessManager::sslErrors, this, [=](QNetworkReply *reply, const QList<QSslError> &errors){
        QList<QSslError> ignore;
        for (const QSslError &error : errors)
        {
            bool noissue = false;
            switch (error.error())
            {
            case QSslError::UnableToGetLocalIssuerCertificate:
            case QSslError::CertificateUntrusted:
                if (nullptr != sslConf && sslConf->isTrusted(reply->sslConfiguration().peerCertificate())) {
                    noissue = true;
                ignore << error;
                }
                break;
            default: break;
            }
            if (!noissue) {
                TERA_LOG(error) << "SSL error: " << error.errorString();
            }
        }
        reply->ignoreSslErrors(ignore);
    });
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

void TimeStamper::sendTSRequest(QByteArray const& timestampRequest, bool test, int retries)
{
    TERA_LOG(debug) << "Connecting to time-server: " << timeserverUrl.toUtf8().constData();
    TERA_LOG(trace) << "Request (in Hex):\n" << timestampRequest.toHex().constData();

    QUrl url(timeserverUrl);
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader(QByteArray("Content-Type"), QByteArray("application/timestamp-query"));

    if (nullptr != sslConf) {
        sslConf->configureRequest(request);
    }

    if (retries > 0) retriesLeft = retries;
    else retriesLeft = 0;

    QNetworkReply* r = nam.post(request, timestampRequest);
    lastPostPtr = r;
    if (!test) {
        lastRequest = timestampRequest;
    } else {
        lastRequest = QByteArray();
    }

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

    if (!testRequest && lastPostPtr != reply) {
        TERA_LOG(error) << "Delayed TS reply. Ignoring";
        return;
    }

    if (QNetworkReply::NoError != reply->error()) {
        QString error;
        error.push_back("Time-stamping request failed: ");
        error.push_back(reply->errorString());
        if (!testRequest && retriesLeft > 0) {
            error.push_back(QString(". Trying to resend data. %1 retries left.").arg(QString::number(retriesLeft)) );
            TERA_LOG(warn) << error;
            sendTSRequest(lastRequest, false, retriesLeft - 1);
            return;
        } else {
            TS_FINISH_DETAILS details = TS_FINISH_DETAILS::OTHER;
            if (QNetworkReply::SslHandshakeFailedError == reply->error()) {
                details = TS_FINISH_DETAILS::SSL_HANDSHAKE_ERROR;
                if (nullptr != sslConf) {
                    error = tr("Couldn't use ID-card for authentication. ") + error;
                }
            }
            notifyClientOnTimestampingFinished(testRequest, false, error, details);
            return;
        }
    }

    QByteArray timeserverResponse = reply->readAll();
    TERA_LOG(trace) << "Time-server response (in Hex):\n" << timeserverResponse.toHex().constData();

// TODO verify response against certificate

    QByteArray timestamp = timeserverResponse;

    if (!extract_timestamp_from_ts_response(timeserverResponse, timestamp)) {
        QString error = "Time-server's response did not contain timestamp.";
        if (!testRequest && retriesLeft > 0) {
            error.push_back(QString(". Trying to resend data. %1 retries left.").arg(QString::number(retriesLeft)) );
            TERA_LOG(warn) << error;
            sendTSRequest(lastRequest, false, retriesLeft - 1);
            return;
        }
        notifyClientOnTimestampingFinished(testRequest, false, error);
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

void TimeStamper::notifyClientOnTimestampingFinished(bool test, bool success, QString errString, TS_FINISH_DETAILS details, QByteArray resp) {
    // TODO bad design
    if (test) {
        emit timestampingTestFinished(success, resp, errString);
    } else {
        emit timestampingFinished(success, errString, details);
    }
}


void TimeStamper::setTimeserverUrl(QString const& url, TimeStamperRequestConfigurationFactory* configurator) {
    sslConf = configurator;
    timeserverUrl = url;
}

void TimeStamper::startTimestamping(QString const& tsUrl, QString const& infile, QString const& outfile) {
    QString errorMsg;
    //if (timeserverUrl != tsUrl) sslConf = NULL; // TODO ???
    timeserverUrl = tsUrl;
    inputFilePath = infile;
    outputFilePath = outfile;

    QByteArray request;
    bool res = getTimestampRequest(request, errorMsg);
    if (!res) {
        emit timestampingFinished(false, errorMsg);
    } else {
        sendTSRequest(request, false, 3);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////

OutputNameGenerator::OutputNameGenerator(QStringList const& inExts, QString const& outExt) {
    setInExts(inExts);
    setOutExt(outExt);
}

QString OutputNameGenerator::getOutFile(QString const& filePath) {
    if (fixedConversion.contains(filePath)) {
        return fixedConversion[filePath];
    }

    QFileInfo fileInfo(filePath);
    QString name = fileInfo.fileName();
    for (int i = 0; i < inExtensions.size(); i++) {
        if (name.endsWith(inExtensions[i])) {
            name = name.left(name.size() - inExtensions[i].size());
            break;
        }
    }

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

void OutputNameGenerator::setInExts(QStringList const& inExts) {
    inExtensions.clear();
    for (QString const& ext : inExts) {
        if (!ext.startsWith(".")) {
            inExtensions << ("." + ext);
        }
        else {
            inExtensions << ext;
        }
    }
}

void OutputNameGenerator::setOutExt(QString const& oe) {
    outExtension = oe;
    if (!outExtension.startsWith(".")) outExtension = "." + outExtension;
}

BatchStamper::BatchStamper(StampingMonitorCallback& mon, OutputNameGenerator& ng, bool end_on_first_fail) :
    monitor(mon), namegen(ng), instaFail(end_on_first_fail), pos(-1)
{
    QObject::connect(this, SIGNAL(triggerNext()),
                     this, SLOT(processNext()));
    QObject::connect(&ts, SIGNAL(timestampingFinished(bool,QString,int)),
                     this, SLOT(timestampFinished(bool,QString,int)));
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
        emit timestampingFinished(FinishingDetails(true, ""));
        return;
    }
    ++pos;
    curIn = input[pos];
    curOut = namegen.getOutFile(curIn);
    if (!monitor.processingFile(curIn, curOut, pos, input.size())) {
        emit timestampingFinished(FinishingDetails::cancelled());
        return;
    }
    ts.startTimestamping(timeServerUrl, curIn, curOut);
}

void BatchStamper::timestampFinished(bool success, QString errString, int i_details) {
    TimeStamper::TS_FINISH_DETAILS details = static_cast<TimeStamper::TS_FINISH_DETAILS>(i_details);
    if (!monitor.processingFileDone(curIn, curOut, pos, input.size(), success, errString)) {
        emit timestampingFinished(FinishingDetails::cancelled());
        return;
    }
    if (!success && (instaFail || TimeStamper::TS_FINISH_DETAILS::SSL_HANDSHAKE_ERROR == details)) {
        emit timestampingFinished(FinishingDetails(success, errString));
    } else {
        emit triggerNext();
    }
}

}
