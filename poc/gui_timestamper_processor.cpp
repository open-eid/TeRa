/*
 * gui_timestamper_processor.cpp
 *
 *  Created on: Nov 17, 2016
 */

#include "gui_timestamper_processor.h"

#include <iostream>

#include <QtAlgorithms>
#include <QStringListModel>
#include <QDebug>

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

    // format
    if (Config::EXTENSION_ASICS == outExt) sw.typeASIC->setChecked(true);
    else sw.typeBDoc->setChecked(true);

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

    if (sw.typeASIC->isChecked()) outExt = Config::EXTENSION_ASICS;
    else outExt = Config::EXTENSION_BDOC;

    GUI2QSet(*sw.modelExclDir, exclDirs);
    GUI2QSet(*sw.modelInclDir, inclDirs);

    previewFiles = sw.cbPreviewFiles->isChecked();
}

void GuiTimestamperProcessor::initializeFilePreviewWindow(FileListWindow& fw) {
    QList<QStandardItem*> list;
    for (int i = 0; i < inFiles.size(); ++i) {
        QString path = inFiles.at(i);
        QStandardItem* item = new QStandardItem(path);
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);
        list.append(item);
    }
    fw.model->clear();
    for (int i = 0; i < list.size(); ++i) {
        fw.model->appendRow(list[i]);
    }
}

void GuiTimestamperProcessor::copySelectedFiles(FileListWindow& fw) {
    QStringList selectedFiles;

    for (int i = 0; i < fw.model->rowCount(); ++i) {
        QStandardItem* item = fw.model->item(i);
        if (NULL == item) continue;
        if (Qt::Checked == item->checkState()) {
            QString filePath = item->text();
            selectedFiles.append(filePath);
        }
    }

    fw.model->clear();
    inFiles = selectedFiles;
}

}
