/*
 * utils.h
 *
 *  Created on: Nov 4, 2016
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <QObject>

namespace ria_tera {

class ExitProgram : public QObject {
    Q_OBJECT
public slots:
    void exitOnFinished(bool success, QString errString);
};

}

#endif /* UTILS_H_ */
