#include <iostream>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QSettings>

#include "utils.h"
#include "logging.h"
#include "disk_crawler.h"
#include "timestamper.h"

namespace {

QString const file_in_param("file_in");
QString const dir_in_param("dir_in");
QString const excl_dir_param("excl_dir");
QString const ts_url_param("ts_url");
QString const file_out_param("file_out"); // TODO ???

}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    ria_tera::initLogging();

    QCommandLineParser parser;

    parser.addHelpOption();
//    parser.addPositionalArgument(file_in_param, "file to be time-stamped");
    parser.addOption(
            QCommandLineOption(ts_url_param,
                    "time server url ex. http://demo.sk.ee/tsa", ts_url_param));
//    parser.addOption(
//            QCommandLineOption(file_out_param,
//                    "output file (default <file_in>.ddoc)", file_out_param));
    parser.addOption(
            QCommandLineOption(dir_in_param,
                    "input directory (*.bdoc files are searched for recursively)", dir_in_param));
    parser.addOption(
            QCommandLineOption(excl_dir_param,
                    "directories to exclude from file search", excl_dir_param));

    parser.process(a);

    QStringList positionals = parser.positionalArguments();
//    if (positionals.isEmpty()) {
//        parser.showHelp();
//    }

    QString file_in("");
//    QString file_in = positionals.at(0);

    QSettings settings("terapoc.ini", QSettings::IniFormat);
    // TODO check unused parameters parameters

    QString file_out;
    QString out_extension("");
    if (parser.isSet(file_out_param)) {
        file_out = parser.value(file_out_param);
    } else {
        out_extension =
                settings.value("output_format", "asics").toString();
        file_out = file_in + "." + out_extension;
    }

    QString time_server_url;
    if (parser.isSet(ts_url_param)) {
        time_server_url = parser.value(ts_url_param);
    } else {
        time_server_url = settings.value("time_server.url").toString();
    }
    time_server_url = time_server_url.trimmed();

    if (time_server_url.isEmpty()) {
        std::cerr << "Time server url not set" << std::endl;
        QCoreApplication::exit(0);
    }

    QString in_dir;
    if (parser.isSet(dir_in_param)) {
        in_dir = parser.value(dir_in_param);
    }

    QStringList excl_dirs = parser.values(excl_dir_param);

    BOOST_LOG_TRIVIAL(info) << "Input file: " << file_in.toUtf8().constData();
    BOOST_LOG_TRIVIAL(info) << "Time-server url: " << time_server_url.toUtf8().constData();
    BOOST_LOG_TRIVIAL(info) << "Output file: " << file_out.toUtf8().constData();

    // TODO one file vs input dir
    ria_tera::DiskCrawler dc(in_dir, excl_dirs);
    QStringList inFiles = dc.crawl(); // TODO if empty
//std::cout << "crawl done" << std::endl;
//    for (int i = 0; i < inFiles.size(); ++i) {
//        QString filePath = inFiles.at(i);
//        std::cout << "(" << (i+1) << "/" << inFiles.size() << "): " << filePath.toUtf8().constData() << std::endl;
//        QString outFilePath = getOutFile(filePath, out_extension);
//        std::cout << "   " << outFilePath.toUtf8().constData() << std::endl;
//    }


    ria_tera::ExitProgram x;
    ria_tera::OutputNameGenerator namegen(out_extension);
    ria_tera::BatchStamper stamper(time_server_url, inFiles, namegen);

    QObject::connect(&stamper, SIGNAL(timestampingFinished(bool,QString)),
                     &x, SLOT(exitOnFinished(bool,QString)), Qt::QueuedConnection);

    stamper.startTimestamping();

//    ria_tera::TimeStamper stamper(file_in, time_server_url, file_out);
//    QObject::connect(&stamper, SIGNAL(timestampingFinished(bool,QString)),
//                     &stamper, SLOT(exitOnFinished(bool,QString)), Qt::QueuedConnection);
//    stamper.startTimestamping();

    return a.exec();
}

