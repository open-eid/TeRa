/*
 * logging.cpp
 *
 *  Created on: Nov 3, 2016
 */
// TODO code-review
#include "logging.h"

#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

//namespace logging = boost::log;

namespace boost
{
// http://stackoverflow.com/questions/6832666/lnk2019-when-including-asio-headers-solution-generated-with-cmake
// http://stackoverflow.com/questions/9272648/boost-symbol-not-found
#ifdef BOOST_NO_EXCEPTIONS
void throw_exception( std::exception const & e ){
    throw 11; // or whatever
};
#endif
}// namespace boost

namespace ria_tera {

void initLogging()
{
    boost::log::add_common_attributes();
    boost::log::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");
    boost::log::add_console_log(
            std::cout,
            boost::log::keywords::format = "[%TimeStamp%] (%Severity%) : %Message%",
            boost::log::keywords::filter = "%Severity% >= boost::log::trivial::info"
//            [](const boost::log::attribute_value_set& attr_set) {
//                return attr_set["Severity"].extract<boost::log::trivial::severity_level>() >= boost::log::trivial::info;
//            }
            );
    boost::log::add_file_log(
            boost::log::keywords::file_name = "terapoc_%Y-%m-%d_%H-%M-%S.%N.log",
            boost::log::keywords::format = "[%TimeStamp%] (%Severity%) : %Message%"
    );
// TODO boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
}

}
