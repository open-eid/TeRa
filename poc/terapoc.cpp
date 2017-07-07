#include <iostream>

// TODO code-review
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QRegExp>
#include <QSettings>
#include <QTimer>

#include "../src/version.h"
#include "../src/cmdtool/cmdline_timestamper_processor.h"

#include "utils.h"
#include "config.h"

namespace {

int const EXIT_CODE_WRONG_ARGUMENTS = 2;

QString const file_in_param("file_in");
QString const dir_in_param("dir_in");
QString const in_dir_recursive_param("R");
QString const file_out_param("file_out");
QString const ext_out_param("ext_out");
QString const excl_dir_param("excl_dir");
QString const no_ini_excl_dirs_param("no_ini_excl_dirs");
QString const ts_url_param("ts_url");
QString const log_level_param("log_level");
QString const logfile_level_param("logfile_level");
QString const logfile_dir_param("logfile_dir");

//#include "terapoc.moc"

}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QString NW_BUG_TEXT("QObject::connect: Cannot connect (null)::stateChanged(QNetworkSession::State) to QNetworkReplyHttpImpl::_q_networkSessionStateChanged(QNetworkSession::State)");
    if (QtWarningMsg == type && NW_BUG_TEXT == msg) return;

    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
#endif
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}

int main(int argc, char *argv[]) {
    qsrand(QTime::currentTime().msec());
    qInstallMessageHandler(myMessageOutput);
    QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");

    QCoreApplication a(argc, argv);
    a.setApplicationVersion(ria_tera::TERA_TOOL_VERSION);

    ria_tera::Config config;

    QCommandLineParser parser;

    parser.addVersionOption();
    parser.addHelpOption();
    parser.setApplicationDescription(
        QString("Input can be either single file (--%1) or whole directory (--%2)\n\n%3 %4").
            arg(file_in_param,
                dir_in_param,
                a.applicationName(),
                a.applicationVersion()));
    // TODO description of .ini file
    parser.addOption(
            QCommandLineOption(file_in_param,
                    "file to be time-stamped", file_in_param));
    parser.addOption(
            QCommandLineOption(dir_in_param,
                    "input directory (*.(" + ria_tera::Config::IN_EXTENSIONS.join(", ") + ") recursiveness can be determined with option '" + in_dir_recursive_param + "')", dir_in_param));
    parser.addOption(
            QCommandLineOption(in_dir_recursive_param,
                    "if set then input directories are searched recursively"));
    QString parTSDefault = (config.getTimeServerURL().isEmpty() ? "ex. http://demo.sk.ee/tsa" : QString("(default %1)").arg(config.getTimeServerURL())); // TODO
    parser.addOption(
            QCommandLineOption(ts_url_param,
                    QString("time server url %1").arg(parTSDefault),
                    ts_url_param));
    parser.addOption(
            QCommandLineOption(ext_out_param,
                    "extension for output file (default '" + ria_tera::Config::DEFAULT_OUT_EXTENSION + "')", ext_out_param));
    parser.addOption(
            QCommandLineOption(file_out_param,
                    "output file, can only be used with --" + file_in_param + " (default <" +
                        file_in_param + ">.<" + ext_out_param + ">)",
                    file_out_param));
    parser.addOption(
            QCommandLineOption(excl_dir_param,
                    "directories to exclude from file search", excl_dir_param));
    parser.addOption(
            QCommandLineOption(no_ini_excl_dirs_param,
                    "if set exclude directories from config file are not taken into account"));

    ria_tera::log_level console_log_lvl = ria_tera::log_level::info;
    ria_tera::log_level file_log_lvl = ria_tera::log_level::trace;
    parser.addOption(
        QCommandLineOption(
            log_level_param,
            QString("console log level, default '%1' (possible values: %2)").arg(log_level_to_string(console_log_lvl), ria_tera::log_level_list()),
            log_level_param));
    parser.addOption(
        QCommandLineOption(
            logfile_level_param,
            QString("logfile log level, default '%1' (possible values: %2)").arg(log_level_to_string(console_log_lvl), ria_tera::log_level_list()),
            logfile_level_param));
    parser.addOption(
        QCommandLineOption(
            logfile_dir_param,
            QString("logfile directory (default is current directory)"),
            logfile_dir_param));

    parser.process(a);

    QString in_dir;
    if (parser.isSet(dir_in_param)) {
        in_dir = parser.value(dir_in_param);
        in_dir = ria_tera::fix_path(in_dir);
        if (in_dir.isEmpty()) {
            std::cout << "<" << QSTR_TO_CCHAR(dir_in_param) << "> can't be empty." << std::endl;
            parser.showHelp(EXIT_CODE_WRONG_ARGUMENTS);
        }
    }

    bool in_dir_recursive = false;
    if (parser.isSet(in_dir_recursive_param)) {
        in_dir_recursive = true;
    }

    QString in_file;
    if (parser.isSet(file_in_param)) {
        in_file = parser.value(file_in_param);
        if (in_file.isEmpty()) {
            std::cout << "<" << QSTR_TO_CCHAR(file_in_param) << "> can't be empty." << std::endl;
            parser.showHelp(EXIT_CODE_WRONG_ARGUMENTS);
        }
    }

    if (parser.isSet(file_in_param) && parser.isSet(dir_in_param)) {
        std::cout << "<" << QSTR_TO_CCHAR(file_in_param) << "> and <"
               << QSTR_TO_CCHAR(dir_in_param) << "> can't be both set." << std::endl;
        parser.showHelp(EXIT_CODE_WRONG_ARGUMENTS);
    }

    if (!parser.isSet(file_in_param) && !parser.isSet(dir_in_param)) {
        std::cout << "<" << QSTR_TO_CCHAR(file_in_param) << "> or <"
                << QSTR_TO_CCHAR(dir_in_param) << "> has to be set." << std::endl;
        parser.showHelp(EXIT_CODE_WRONG_ARGUMENTS);
    }

    if (!in_file.isEmpty() && !QFileInfo(in_file).exists()) {
        std::cout << "Input file '" << QSTR_TO_CCHAR(in_file) << "' does not exist." << std::endl;
        return EXIT_CODE_WRONG_ARGUMENTS;
    }

    if (!in_dir.isEmpty() && !QFileInfo(in_dir).exists()) {
        std::cout << "Input directory '" << QSTR_TO_CCHAR(in_dir) << "' does not exist." << std::endl;
        return EXIT_CODE_WRONG_ARGUMENTS;
    }

    if (parser.isSet(file_out_param) && !parser.isSet(file_in_param)) {
        std::cout << "<" << QSTR_TO_CCHAR(file_out_param) << "> can only be set with <"
                << QSTR_TO_CCHAR(file_in_param) << "> has to be set." << std::endl;
        parser.showHelp(EXIT_CODE_WRONG_ARGUMENTS);
    }

    if (parser.isSet(file_out_param) && parser.isSet(ext_out_param)) {
        std::cout << "<" << QSTR_TO_CCHAR(file_out_param) << "> and <"
                << QSTR_TO_CCHAR(ext_out_param) << "> can't be both set." << std::endl;
        parser.showHelp(EXIT_CODE_WRONG_ARGUMENTS);
    }

    if (parser.isSet(log_level_param)) {
        QString lvl_str = parser.value(log_level_param);
        if (!log_level_from_string(lvl_str, console_log_lvl)) {
            QString err = QString("Illegal '%1' value '%2' (allowed values: %3)").
                arg(log_level_param, lvl_str, ria_tera::log_level_list());
            std::cout << err.toUtf8().constData() << std::endl;
            std::cout << std::endl;
            parser.showHelp(EXIT_CODE_WRONG_ARGUMENTS);
        }
    }

    if (parser.isSet(logfile_level_param)) {
        QString lvl_str = parser.value(logfile_level_param);
        if (!log_level_from_string(lvl_str, file_log_lvl)) {
            QString err = QString("Illegal '%1' value '%2' (allowed values: %3)").
                arg(logfile_level_param, lvl_str, ria_tera::log_level_list());
            std::cout << err.toUtf8().constData() << std::endl;
            std::cout << std::endl;
            parser.showHelp(EXIT_CODE_WRONG_ARGUMENTS);
        }
    }

    // set-up logging
    ria_tera::logger.addConsoleLog(console_log_lvl);
    QString logfile_dir;
    if (parser.isSet(logfile_dir_param)) {
        logfile_dir = parser.value(logfile_dir_param);
    }

    if (!ria_tera::logger.addFileLog(file_log_lvl, logfile_dir)) {
        std::cout << QString("Add '--%1 %2' to disable logging to a file or use '--%3 <path>' to set directory for logfile.").
            arg(logfile_level_param,
                log_level_to_string(ria_tera::log_level::none),
                logfile_dir_param).toUtf8().constData() << std::endl;
        return EXIT_CODE_WRONG_ARGUMENTS;
    }

    QString out_extension("");
    if (parser.isSet(ext_out_param)) {
        out_extension = parser.value(ext_out_param);
        if (!QRegExp("[a-zA-Z\\d._]+").exactMatch(out_extension)) {
            std::cout << "Illegal output file extension set in command line '" << QSTR_TO_CCHAR(out_extension) << "'" << std::endl;
            return EXIT_CODE_WRONG_ARGUMENTS;
        }
    } else {
        out_extension = config.getOutExtension();
        if (!QRegExp("[a-zA-Z\\d._]+").exactMatch(out_extension)) {
            std::cout << "Illegal output file extension set in configuration file '" << QSTR_TO_CCHAR(out_extension) << "'" << std::endl;
            return EXIT_CODE_WRONG_ARGUMENTS;
        }
    }

    QString file_out;
    if (parser.isSet(file_in_param)) {
        if (parser.isSet(file_out_param)) {
            file_out = parser.value(file_out_param);
        } else {
            ria_tera::OutputNameGenerator namegen(ria_tera::Config::IN_EXTENSIONS, out_extension);
            file_out = namegen.getOutFile(in_file);
        }
    }

    QString time_server_url;
    if (parser.isSet(ts_url_param)) {
        time_server_url = parser.value(ts_url_param);
    } else {
        time_server_url = config.getTimeServerURL();
    }
    time_server_url = time_server_url.trimmed();

    if (time_server_url.isEmpty()) {
        std::cerr << "Time server url not set" << std::endl;
        return EXIT_CODE_WRONG_ARGUMENTS;
    }

    QSet<QString> excl_dirs_set;

    QStringList ex= parser.values(excl_dir_param);
    for (int i = 0; i < ex.size(); ++i)
        ria_tera::Config::append_excl_dirs(ex.at(i), excl_dirs_set);

    if (!parser.isSet(no_ini_excl_dirs_param)) {
        excl_dirs_set.unite(config.getExclDirsXXXXXXXX());
    }

    // log output
    if (!in_file.isEmpty()) {
        TERA_COUT("Parameter - input-file: " << QSTR_TO_CCHAR(in_file));
    }
    if (!in_dir.isEmpty()) {
        if (in_dir_recursive) {
            TERA_COUT("Parameter - input-directory (recursive): " << QSTR_TO_CCHAR(in_dir));
        } else {
            TERA_COUT("Parameter - input-directory (non-recursive): " << QSTR_TO_CCHAR(in_dir));
        }
    }
    TERA_COUT("Parameter - time-server url: " << QSTR_TO_CCHAR(time_server_url));

    if (!file_out.isEmpty()) {
        TERA_COUT("Parameter - Output file: " << file_out.toUtf8().constData());
    }
    QStringList excl_dirs = excl_dirs_set.values();
    for (int i = 0; i < excl_dirs.size(); ++i) {
        TERA_COUT("Parameter - exclude-directory: " << excl_dirs[i].toUtf8().constData());
    }

    ///////////////////////////////////////////////////////////////////////////
    ria_tera::TeRaMonitor::IOParameters ioparams;
    ioparams.out_extension    = out_extension;
    ioparams.in_file          = in_file;
    ioparams.excl_dirs        = excl_dirs;
    ioparams.in_dir           = in_dir;
    ioparams.in_dir_recursive = in_dir_recursive;
    ioparams.file_out         = file_out;

    ria_tera::TeRaMonitor monitor;
    monitor.kickstart(time_server_url, ioparams);

    return a.exec();
}

