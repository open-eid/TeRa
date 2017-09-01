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

bool TeraLogger::addFileLog(log_level lvl, QString dir_path) {
    file_level = lvl;
    if (lvl == log_level::none) return true;

    QString error;
    QDir dir = QDir::current();
    if (!dir_path.isEmpty()) {
        dir.setPath(dir_path);
    }
    QString const filePrefix = "tera_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss-zzz");
    QString const fileSufix = ".log";

    LogFile* log = LogFile::openLogFile(dir, filePrefix, fileSufix, error);
    if (log) {
        logfile.reset(log);
        TeraLoggerLine(log_level::info) << QString("Opened log file '%1'").arg(log->filePath()).toUtf8().constData();
        return true;
    } else {
        TeraLoggerLine logline(log_level::error);
        logline.setConsoleOnly();
        logline.append(error.toUtf8().constData());
        return false;
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

TeraLoggerLine& TeraLoggerLine::operator<<(QString const& text) {
    if (text.isNull()) append("<null>");
    else append(text.toUtf8());
    return *this;
}

TeraLoggerLine& TeraLoggerLine::operator<<(QByteArray const& text) {
    if (text.isNull()) append("<null>");
    else append(text.constData());
    return *this;
}

TeraLoggerLine& TeraLoggerLine::operator<<(char const* text) {
    append(text);
    return *this;
}

TeraLoggerLine& TeraLoggerLine::operator<<(int nr) {    *this << QString::number(nr).toUtf8().constData();    return *this;}

QString log_level_to_string(log_level lvl) {
    switch (lvl) {
    case none:  return "none";
    case error: return "error";
    case warn:  return "warn";
    case info:  return "info";
    case debug: return "debug";
    case trace: return "trace";
    default:
        return "XXX";
    }
}

bool log_level_from_string(QString const& trace, log_level& lvl) {
    if (trace == "none") {
        lvl = log_level::none;
        return true;
    }
    if (trace == "error") {
        lvl = log_level::error;
        return true;
    }
    if (trace == "warn") {
        lvl = log_level::warn;
        return true;
    }
    if (trace == "info") {
        lvl = log_level::info;
        return true;
    }
    if (trace == "debug") {
        lvl = log_level::debug;
        return true;
    }
    if (trace == "trace") {
        lvl = log_level::trace;
        return true;
    }
    return false;
}

QString log_level_list() {
    return "none, error, warn, info, debug, trace";
}





}
