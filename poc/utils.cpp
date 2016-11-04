/*
 * utils.cpp
 *
 *  Created on: Nov 4, 2016
 */

#include "utils.h"

#include <QCoreApplication>

#include "logging.h"

namespace ria_tera {

void ExitProgram::exitOnFinished(bool success, QString errString) {
    if (success) {
        BOOST_LOG_TRIVIAL(info) << "Timestamping finished successfully :)";
    } else {
        BOOST_LOG_TRIVIAL(error) << "Timestamping failed: " << errString.toUtf8().constData();
    }
    QCoreApplication::exit(0);
}

}
