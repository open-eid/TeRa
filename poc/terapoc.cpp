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
#include "logging.h"
#include "disk_crawler.h"
#include "timestamper.h"

namespace {

int const EXIT_CODE_WRONG_ARGUMENTS = 2;

QString const file_in_param("file_in");
QString const dir_in_param("dir_in");
QString const file_out_param("file_out");
QString const ext_out_param("ext_out");
QString const excl_dir_param("excl_dir");
QString const no_ini_excl_dirs_param("no_ini_excl_dirs");
QString const ts_url_param("ts_url");

QString const INI_FILE_NAME("terapoc.ini");
QString const INI_GROUP("tera");
QString const INI_GROUP_ = INI_GROUP + "/";

QString const default_out_extension("asics");

#define TERA_COUT(xxx) {\
    BOOST_LOG_TRIVIAL(info) << xxx;\
    std::cout << xxx << std::endl;\
    };

class TeRaMonitor : public ria_tera::ProcessingMonitorCallback {
public:
    // TODO logging
    void virtual processingPath(QString const& path) {
        TERA_COUT("Searching " << path.toUtf8().constData());
    };
    void virtual excludingPath(QString const& path) {
        TERA_COUT("   Excluding " << path.toUtf8().constData());
    };
    void virtual foundFile(QString const& path) {
        TERA_COUT("   Found " << path.toUtf8().constData());
    };
    void virtual processingFile(QString const& pathIn, QString const& pathOut, int nr, int totalCnt) {
        TERA_COUT("Timestamping (" << (nr+1) << "/" << totalCnt << ") " << pathIn.toUtf8().constData() <<
                " -> " << pathOut.toUtf8().constData());
    };
};

}

static void append_excl_dirs(QString const& val, QStringList& excl_dirs, QSet<QString>& excl_dirs_set) {
    QStringList paths = val.split(":", QString::SkipEmptyParts);
    for (int i = 0; i < paths.size(); ++i) {
        QString path = ria_tera::fix_path(paths.at(i));
        if (!excl_dirs_set.contains(path)) {
            excl_dirs_set.insert(path);
            excl_dirs.append(path);
        }
    }
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
                    "input directory (*.ddoc files are searched for recursively)", dir_in_param));
    parser.addOption(
            QCommandLineOption(ts_url_param,
                    "time server url ex. http://demo.sk.ee/tsa", ts_url_param));
    parser.addOption(
            QCommandLineOption(ext_out_param,
                    "extension for output file (default '" + default_out_extension + "')", ext_out_param));
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

    QSettings settings(INI_FILE_NAME, QSettings::IniFormat);
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
        out_extension =
                settings.value("output_format", default_out_extension).toString();
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
            ria_tera::OutputNameGenerator namegen(out_extension);
            file_out = namegen.getOutFile(in_file);
        }
    }

    QString time_server_url;
    if (parser.isSet(ts_url_param)) {
        time_server_url = parser.value(ts_url_param);
    } else {
        time_server_url = settings.value(INI_GROUP_ + "time_server.url").toString();
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
        append_excl_dirs(ex.at(i), excl_dirs, excl_dirs_set);

    if (!parser.isSet(no_ini_excl_dirs_param)) {
        QString const key = INI_GROUP_ + "excl_dir";
        if (settings.contains(key)) {
            QString val = settings.value(key).toString();
            append_excl_dirs(val, excl_dirs, excl_dirs_set);
        }

        QString const key_ = key + ".";
        for (int i = 0; i < settingsKeys.length(); ++i) {
            QString k = settingsKeys.at(i);
            if (k.startsWith(key_)) {
                QString val = settings.value(k).toString();
                append_excl_dirs(val, excl_dirs, excl_dirs_set);
            }
        }
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

    TeRaMonitor monitor;

    ria_tera::ExitProgram x;
    ria_tera::OutputNameGenerator namegen(out_extension);

    QStringList inFiles;
    if (in_file.isEmpty()) {
        ria_tera::DiskCrawler dc(monitor, in_dir, excl_dirs);
        inFiles = dc.crawl();
        // TODO if empty
    } else {
        inFiles.append(in_file);
        namegen.setFixedOutFile(in_file, file_out);
    }

    ria_tera::BatchStamper stamper(monitor, time_server_url, inFiles, namegen);

    QObject::connect(&stamper, SIGNAL(timestampingFinished(bool,QString)),
                     &x, SLOT(exitOnFinished(bool,QString)), Qt::QueuedConnection);

    stamper.startTimestamping();

//    ria_tera::TimeStamper stamper(file_in, time_server_url, file_out);
//    QObject::connect(&stamper, SIGNAL(timestampingFinished(bool,QString)),
//                     &stamper, SLOT(exitOnFinished(bool,QString)), Qt::QueuedConnection);
//    stamper.startTimestamping();

    return a.exec();
}

