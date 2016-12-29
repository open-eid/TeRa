/*
 * gui_timestamper_processor.cpp
 *
 *  Created on: Nov 17, 2016
 */

#include "gui_timestamper_processor.h"

#include <iostream>

#include <QtAlgorithms>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QStringListModel>
#include <QTemporaryFile>

#include "utils.h"

namespace ria_tera {

GuiTimestamperProcessor::GuiTimestamperProcessor() {
    timeServerUrl = config.readTimeServerURL();

    outExt = config.readOutExtension();
    if (outExt != Config::EXTENSION_BDOC) outExt = Config::EXTENSION_ASICS;

    exclDirs.unite(config.readExclDirs());

    inclDirs.unite(config.getDefaultInclDirs());

    previewFiles = false;
}

void QSet2GUI(QSet<QString> const& set, QStringListModel& model) {
    QList<QString> l = set.toList();
    qSort(l);
    model.setStringList(l);
}

void GuiTimestamperProcessor::initializeSettingsWindow(TeraSettingsWin& sw) {
    while (sw.tabWidget->count() > 3) {
        sw.tabWidget->removeTab(3);
    }

    // time server url
    sw.lineTSURL->setText(timeServerUrl);

    // exclude dirs
    QSet2GUI(exclDirs, *sw.modelExclDir);

    // include dirs
    QSet2GUI(inclDirs, *sw.modelInclDir);

    // preview files
    sw.cbPreviewFiles->setChecked(previewFiles);
}

void GUI2QSet(QStringListModel const& model, QSet<QString>& set) {
    set = QSet<QString>::fromList(model.stringList());
}

void GuiTimestamperProcessor::readSettings(TeraSettingsWin& sw) {
    timeServerUrl = sw.lineTSURL->text().trimmed();

    GUI2QSet(*sw.modelExclDir, exclDirs);
    GUI2QSet(*sw.modelInclDir, inclDirs);

    previewFiles = sw.cbPreviewFiles->isChecked();
}

void GuiTimestamperProcessor::initializeFilePreviewWindow(FileListWindow& fw) {
    fw.setFileList(inFiles);
}

void GuiTimestamperProcessor::copySelectedFiles(FileListWindow& fw) {
    inFiles = fw.extractSelectedFileList();
}

bool GuiTimestamperProcessor::openLogFile(QString& errorText) {
    logfile.reset();

    QDir dir = QDir::home();
    QString file_prefix = "tera_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz");
    QString file_sufix = ".log";

    QFileInfo fileinfo(dir, file_prefix + file_sufix);
    int nr = 0;
    while (fileinfo.exists()) {
        fileinfo.setFile(dir, file_prefix + "(" + QString::number(++nr) + ")" + file_sufix);
    }

    QScopedPointer<QFile> file(new QFile(fileinfo.absoluteFilePath()));
    if (file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Unbuffered)) {
        logfile.reset(new LogFile(file.take()));
        return true;
    } else {
        QString path = fileinfo.absoluteFilePath();
        errorText = tr("Error opening log file") + " '" + path + "'";
        if (QFileDevice::PermissionsError == file->error()) {
            errorText += ": " + tr("No permissions");
        } else if (QFileDevice::ResourceError == file->error()) {
            errorText += ": " + tr("Out of resources");
        }
        return false;
    }
}

GuiTimestamperProcessor::LogFile::LogFile(QFile* file) {
    logFile.reset(file);

    logStream.setCodec("UTF-8");
    logStream.setDevice(logFile.data());
}

GuiTimestamperProcessor::LogFile::~LogFile() {
    close();
}

QString GuiTimestamperProcessor::LogFile::filePath() {
    if (logFile) {
        return logFile->fileName();
    } else {
        return QString();
    }
}

void GuiTimestamperProcessor::LogFile::close() {
    logStream.flush();
    if (logFile.data()) {
        logFile->close();
    }
}

}
