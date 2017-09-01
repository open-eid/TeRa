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
#ifndef LOGGING_H_
#define LOGGING_H_

#include <QDir>
#include <QFile>
#include <QTextStream>

// info trace error
#define TERA_LOG(level) ria_tera::TeraLoggerLine(ria_tera::log_level:: level)

#define TERA_COUT(xxx) {TERA_LOG(info) << xxx;};

namespace ria_tera {

enum log_level {none=0, error, warn, info, debug, trace};

QString log_level_to_string(log_level lvl);
bool log_level_from_string(QString const& lvl_str, log_level& lvl);
QString log_level_list();

class LogFile {
public:
    LogFile(QFile* file);
    virtual ~LogFile();
    QString filePath();
    QTextStream& getStream() { return logStream; };
    void close();

    static LogFile* openLogFile(QDir const& dir, QString const& file_prefix, QString const& file_sufix, QString& error);
private:
    QScopedPointer<QFile> logFile;
    QTextStream logStream;
};

class TeraLogger {
public:
    TeraLogger();
    ~TeraLogger();
    void addConsoleLog(log_level lvl);
    bool addFileLog(log_level lvl, QString dir_path = QString());
    void append(log_level lvl, char const* text, bool consoleOnly = false);
private:
    log_level console_level;
    log_level file_level;
    QScopedPointer<LogFile> logfile;
};

class TeraLoggerLine {
public:
    TeraLoggerLine(log_level lvl);
    ~TeraLoggerLine();
    void append(char const* text);
    void setConsoleOnly();
    TeraLoggerLine& operator<<(QString const& text);
    TeraLoggerLine& operator<<(QByteArray const& text);
    TeraLoggerLine& operator<<(char const* text);
    TeraLoggerLine& operator<<(int nr);
private:
    bool consoleOnly;
    log_level level;
    QString message;
};

extern TeraLogger logger;

}

#endif /* LOGGING_H_ */
