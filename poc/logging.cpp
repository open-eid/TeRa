// TODO code-review
#include "logging.h"

#include <iostream>

#include <QDateTime>
#include <QDebug>

namespace ria_tera {

TeraLogger logger;

//============================== LogFile ==============================

LogFile::LogFile(QFile* file) {
    logFile.reset(file);

    logStream.setCodec("UTF-8");
    logStream.setDevice(logFile.data());
}

LogFile::~LogFile() {
    close();
}

QString LogFile::filePath() {
    if (logFile) {
        return logFile->fileName();
    } else {
        return QString();
    }
}

void LogFile::close() {
    logStream.flush();
    if (logFile.data()) {
        logFile->close();
    }
}

LogFile* LogFile::openLogFile(QDir const& dir, QString const& file_prefix, QString const& file_sufix, QString& error) {
    QFileInfo fileinfo(dir, file_prefix + file_sufix);
    int nr = 0;
    while (fileinfo.exists()) {
        fileinfo.setFile(dir, file_prefix + "(" + QString::number(++nr) + ")" + file_sufix);
    }

    QScopedPointer<QFile> file(new QFile(fileinfo.absoluteFilePath()));
    if (file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Unbuffered)) {
        return new LogFile(file.take());
    } else {
        QString path = fileinfo.absoluteFilePath();
        QString errorText = QString() + "Error opening log file" + " '" + path + "'";
        if (QFileDevice::PermissionsError == file->error()) {
            errorText += QString() + ": " + "No permissions";
        }
        else if (QFileDevice::ResourceError == file->error()) {
            errorText += QString() + ": " + "Out of resources";
        }
        error = errorText;
        return NULL;
    }
}

//============================== TeraLogger ==============================

TeraLogger::TeraLogger() : console_level(log_level::none), file_level(log_level::none) {}

TeraLogger::~TeraLogger() {
    if (!logfile.isNull()) logfile->close();
}

void TeraLogger::addConsoleLog(log_level lvl) { console_level = lvl; }

void TeraLogger::addFileLog(log_level lvl) {
    file_level = lvl;
    if (lvl == log_level::none) return;

    QString error;
    QDir dir = QDir::current();
    QString const filePrefix = "terapoc_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss-zzz");
    QString const fileSufix = ".log";

    LogFile* log = LogFile::openLogFile(dir, filePrefix, fileSufix, error);
    if (log) {
        logfile.reset(log);
        TeraLoggerLine(log_level::info) << QString("Opened log file '%1'").arg(log->filePath()).toUtf8().constData();
    } else {
        TeraLoggerLine logline(log_level::error);
        logline.setConsoleOnly();
        logline.append(error.toUtf8().constData());
    }
}

void TeraLogger::append(log_level lvl, char const* text, bool consoleOnly) {
    if (NULL == text) text = "NULL";

    if (console_level != log_level::none && lvl <= console_level) {
        std::cout << text;
    }

    if (!consoleOnly && !logfile.isNull() && file_level != log_level::none && lvl <= file_level) {
        logfile->getStream() << text;
        logfile->getStream().flush();
    }
}

//============================== TeraLoggerLine ==============================

TeraLoggerLine::TeraLoggerLine(log_level lvl) : consoleOnly(false), level(lvl) {}

TeraLoggerLine::~TeraLoggerLine() {
    QString datetime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString levelStr;
    switch (level) {
    case log_level::none: levelStr = "none"; break;
    case log_level::error: levelStr = "error"; break;
    case log_level::warn: levelStr = "warn"; break;
    case log_level::info: levelStr = "info"; break;
    case log_level::debug: levelStr = "debug"; break;
    case log_level::trace: levelStr = "trace"; break;
    default: levelStr = "???";
    }

    QString logLine = QString("[%1] (%2) : %3\n").arg(datetime, levelStr, message);
    logger.append(level, logLine.toUtf8().constData());
}

void TeraLoggerLine::append(char const* text) { message += text; }

void TeraLoggerLine::setConsoleOnly() { consoleOnly = true; };

TeraLoggerLine& TeraLoggerLine::operator<<(char const* text) {
    append(text);
    return *this;
}

TeraLoggerLine& TeraLoggerLine::operator<<(int nr) {    *this << QString::number(nr).toUtf8().constData();    return *this;}
void initLogging()
{
    logger.addConsoleLog(log_level::info);
    logger.addFileLog(log_level::trace);
}

}
