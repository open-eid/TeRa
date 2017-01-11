// TODO ll
#ifndef LOGGING_H_
#define LOGGING_H_

#include <QDir>
#include <QFile>
#include <QTextStream>

// info trace error
#define TERA_LOG(level) ria_tera::TeraLoggerLine(ria_tera::log_level:: level)

namespace ria_tera {

enum log_level {none=0, error, warn, info, debug, trace};

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
    void addFileLog(log_level lvl);
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
private:
    bool consoleOnly;
    log_level level;
    QString message;
};

TeraLoggerLine& operator<<(TeraLoggerLine& log, char const* text);
TeraLoggerLine& operator<<(TeraLoggerLine& log, int nr);

extern TeraLogger logger;

void initLogging();

}

#endif /* LOGGING_H_ */
