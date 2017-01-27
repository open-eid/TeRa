/*
* xxx.cpp
*/

#include <iostream>

#include <QApplication>
#include <QLoggingCategory>
#include <QThreadPool>
#include <QUrl>
#include <QJsonObject>

#include "src/libdigidoc/Configuration.h"

class tricky : public QObject {
    Q_OBJECT
public:
public slots:
    void configFinished(bool changed, const QString &err);
};

#include "xxx.moc"

void tricky::configFinished(bool changed, const QString &err) {
    qDebug() << QString("tricky::configFinished %1 '%2'").arg(QString(changed), err);
    QJsonObject jo = Configuration::instance().object();
    for (auto it = jo.begin(); it != jo.end(); ++it) {
        qDebug() << QString("   [%1] -> '%2'").arg(it.key(), it.value().toString());
    }
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    qDebug() << "Start...";
    tricky t;
    QObject::connect(&Configuration::instance(), &Configuration::finished, &t, &tricky::configFinished);

    QString CONFIG_URL = "https://id.eesti.ee/config.json";
    Configuration::instance().update();

    qDebug() << "exec...";
    return a.exec();
}


