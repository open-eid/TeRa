#include <iostream>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QSettings>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include "timestamper.h"

namespace {

QString const file_in_param("file_in");
QString const ts_url_param("ts_url");
QString const file_out_param("file_out");

}

static void initLogging()
{
    boost::log::add_common_attributes();
    boost::log::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");
    boost::log::add_console_log(
            std::cout,
            boost::log::keywords::format = "[%TimeStamp%] (%Severity%) : %Message%"
            );
    boost::log::add_file_log(
            boost::log::keywords::file_name = "terapoc_%Y-%m-%d_%H-%M-%S.%N.log",
            boost::log::keywords::format = "[%TimeStamp%] (%Severity%) : %Message%"
    );
// TODO boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    initLogging();

    QCommandLineParser parser;

    parser.addHelpOption();
    parser.addPositionalArgument(file_in_param, "file to be time-stamped");
    parser.addOption(
            QCommandLineOption(ts_url_param,
                    "time server url ex. http://demo.sk.ee/tsa"));
    parser.addOption(
            QCommandLineOption(file_out_param,
                    "output file (default <file_in>.ddoc)"));

    parser.process(a);

    QStringList positionals = parser.positionalArguments();
    if (positionals.isEmpty()) {
        parser.showHelp();
    }

    QString file_in = positionals.at(0);

    QSettings settings("terapoc.ini", QSettings::IniFormat);
    // TODO check unused parameters parameters

    QString file_out;
    if (parser.isSet(file_out_param)) {
        file_out = parser.value(file_out_param);
    } else {
        QString out_extension =
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

    BOOST_LOG_TRIVIAL(info) << "Input file: " << file_in.toUtf8().constData();
    BOOST_LOG_TRIVIAL(info) << "Time-server url: " << time_server_url.toUtf8().constData();
    BOOST_LOG_TRIVIAL(info) << "Output file: " << file_out.toUtf8().constData();

    ria_tera::TimeStamper stamper(file_in, time_server_url, file_out);
    QObject::connect(&stamper, SIGNAL(timestampingFinished(bool,QString)),
                     &stamper, SLOT(exitOnFinished(bool,QString)), Qt::QueuedConnection);
    stamper.startTimestamping();

    return a.exec();
}

