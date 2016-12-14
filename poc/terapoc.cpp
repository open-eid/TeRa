#include <iostream>

// TODO code-review
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QRegExp>
#include <QSettings>

#include "utils.h"
#include "config.h"
#include "logging.h"
#include "disk_crawler.h"
#include "timestamper.h"

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

#define TERA_COUT(xxx) {\
    BOOST_LOG_TRIVIAL(info) << xxx;\
    std::cout << xxx << std::endl;\
    };

class TeRaMonitor : public ria_tera::ProcessingMonitorCallback {
public:
    // TODO logging
    bool processingPath(QString const& path, double progress_percent) {
        TERA_COUT("Searching " << path.toUtf8().constData());
        return true;
    };
    bool excludingPath(QString const& path) {
        TERA_COUT("   Excluding " << path.toUtf8().constData());
        return true;
    };
    bool foundFile(QString const& path) {
        TERA_COUT("   Found " << path.toUtf8().constData());
        return true;
    };
    bool processingFile(QString const& pathIn, QString const& pathOut, int nr, int totalCnt) {
        TERA_COUT("Timestamping (" << (nr+1) << "/" << totalCnt << ") " << pathIn.toUtf8().constData() <<
                " -> " << pathOut.toUtf8().constData());
        return true;
    };
    bool processingFileDone(QString const& pathIn, QString const& pathOut, int nr, int totalCnt, bool success, QString const& errString) {return true;};
};

}

int main(int argc, char *argv[]) {
    QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");

    QCoreApplication a(argc, argv);
    ria_tera::initLogging();

    QCommandLineParser parser;

    parser.addHelpOption();
    parser.setApplicationDescription(QString() +
            "Input can be either single file (--" + file_in_param + ") or whole directory (--" + dir_in_param +  ")");
    // TODO description of .ini file
    parser.addOption(
            QCommandLineOption(file_in_param,
                    "file to be time-stamped", file_in_param));
    parser.addOption(
            QCommandLineOption(dir_in_param,
                    "input directory (*." + ria_tera::Config::EXTENSION_IN + " files are searched for recursively)", dir_in_param));
    parser.addOption(
            QCommandLineOption(ts_url_param,
                    "time server url ex. http://demo.sk.ee/tsa", ts_url_param));
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

    ria_tera::Config config;
    QSettings& settings(config.getInternalSettings());
    QStringList settingsKeys = settings.allKeys();
    // TODO check unused parameters parameters

    QString out_extension("");
    if (parser.isSet(ext_out_param)) {
        out_extension = parser.value(ext_out_param);
        if (!QRegExp("[a-zA-Z\\d._]+").exactMatch(out_extension)) {
            std::cout << "Illegal output file extension set in command line '" << QSTR_TO_CCHAR(out_extension) << "'" << std::endl;
            return EXIT_CODE_WRONG_ARGUMENTS;
        }
    } else {
        out_extension = config.readOutExtension();
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
            ria_tera::OutputNameGenerator namegen(ria_tera::Config::EXTENSION_IN, out_extension);
            file_out = namegen.getOutFile(in_file);
        }
    }

    QString time_server_url;
    if (parser.isSet(ts_url_param)) {
        time_server_url = parser.value(ts_url_param);
    } else {
        time_server_url = config.readTimeServerURL();
    }
    time_server_url = time_server_url.trimmed();

    if (time_server_url.isEmpty()) {
        std::cerr << "Time server url not set" << std::endl;
        return EXIT_CODE_WRONG_ARGUMENTS;
    }

    QStringList excl_dirs;
    QSet<QString> excl_dirs_set;

    QStringList ex= parser.values(excl_dir_param);
    for (int i = 0; i < ex.size(); ++i)
        ria_tera::Config::append_excl_dirs(ex.at(i), excl_dirs_set);

    if (!parser.isSet(no_ini_excl_dirs_param)) {
        excl_dirs_set.unite(config.readExclDirs());
    }

    // log output
    if (!in_file.isEmpty()) {
        TERA_COUT("Parameter - input-file: " << QSTR_TO_CCHAR(in_file));
    }
    if (!in_dir.isEmpty()) {
        TERA_COUT("Parameter - input-directory: " << QSTR_TO_CCHAR(in_dir));
    }
    TERA_COUT("Parameter - time-server url: " << QSTR_TO_CCHAR(time_server_url));

    if (!file_out.isEmpty()) {
        TERA_COUT("Parameter - Output file: " << file_out.toUtf8().constData());
    }
    for (int i = 0; i < excl_dirs.size(); ++i) {
        TERA_COUT("Parameter - exclude-directory: " << excl_dirs[i].toUtf8().constData());
    }

    // TODO test network

    TeRaMonitor monitor;

    ria_tera::ExitProgram x;
    ria_tera::OutputNameGenerator namegen(ria_tera::Config::EXTENSION_IN, out_extension);

    QStringList inFiles;
    if (in_file.isEmpty()) {
        ria_tera::DiskCrawler dc(monitor, ria_tera::Config::EXTENSION_IN);
        dc.addExcludeDirs(excl_dirs);
        dc.addInputDir(in_dir, in_dir_recursive);
        inFiles = dc.crawl();
        // TODO if empty
    } else {
        inFiles.append(in_file);
        namegen.setFixedOutFile(in_file, file_out);
    }

    if (0 == inFiles.size()) {
        TERA_COUT("No *." << QSTR_TO_CCHAR(ria_tera::Config::EXTENSION_IN) << " files found.");
    }
    ria_tera::BatchStamper stamper(monitor, namegen);

    QObject::connect(&stamper, SIGNAL(timestampingFinished(bool,QString)),
                     &x, SLOT(exitOnFinished(bool,QString)), Qt::QueuedConnection);

    stamper.startTimestamping(time_server_url, inFiles); // TODO error to XXX when network is down for example

    return a.exec();
}

