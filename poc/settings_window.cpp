/*
 * settings_window.cpp
 *
 *  Created on: Nov 18, 2016
 */

#include "settings_window.h"

#include <iostream>
#include <QFileDialog>

#include "utils.h"

namespace ria_tera {

TeraSettingsWin::TeraSettingsWin(QWidget *parent) :
        QDialog(parent)
{
    setupUi(this);
    tabWidget->setCurrentIndex(0);

    modelExclDir = new QStringListModel(); // TODO delete?
    listExclDir->setModel(modelExclDir);

    modelInclDir = new QStringListModel(); // TODO delete?
    listInclDir->setModel(modelInclDir);

    connect(btnAddExclDir, SIGNAL (clicked()), this, SLOT (handleAddExclDir()), Qt::QueuedConnection);
    connect(btnDelExclDir, SIGNAL (clicked()), this, SLOT (handleDelExclDir()), Qt::QueuedConnection);
    connect(btnExclDirSearch, SIGNAL (clicked()), this, SLOT (handleExclDirSearch()), Qt::QueuedConnection);

    connect(btnAddInclDir, SIGNAL (clicked()), this, SLOT (handleAddInclDir()), Qt::QueuedConnection);
    connect(btnDelInclDir, SIGNAL (clicked()), this, SLOT (handleDelInclDir()), Qt::QueuedConnection);
    connect(btnInclDirSearch, SIGNAL (clicked()), this, SLOT (handleInclDirSearch()), Qt::QueuedConnection);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

TeraSettingsWin::~TeraSettingsWin() {
    ;
}

void TeraSettingsWin::handleAddExclDir() {
    addDirToList(lineExclDir, modelExclDir);
}

void TeraSettingsWin::handleDelExclDir() {
    deleteSelected(listExclDir);
}

void TeraSettingsWin::handleExclDirSearch() {
    searchDir(this, lineExclDir);
}

void TeraSettingsWin::handleAddInclDir() {
    addDirToList(lineInclDir, modelInclDir);
}

void TeraSettingsWin::handleDelInclDir() {
    deleteSelected(listInclDir);
}

void TeraSettingsWin::handleInclDirSearch() {
    searchDir(this, lineInclDir);
}

void TeraSettingsWin::addDirToList(QLineEdit* line, QStringListModel* model) {
    QString newPath = line->text().trimmed();
    if (newPath.isEmpty()) return; // TODO

    int row = 0;
    while (row < model->rowCount()) {
        QModelIndex index = model->index(row);
        QString data = model->data(index, Qt::DisplayRole).toString();
        if (data == newPath) return; // TODO
        if (data > newPath) break;
        ++row;
    }
    model->insertRow(row);
    QModelIndex index = model->index(row);
    model->setData(index, newPath);

    line->setText("");
}

void TeraSettingsWin::deleteSelected(QListView* lv) {
    QModelIndexList indexes = lv->selectionModel()->selectedIndexes();
    qSort(indexes);
    for (int i = indexes.count()-1; i >= 0; --i) {
        lv->model()->removeRow(indexes.at(i).row());
    }
}

void TeraSettingsWin::searchDir(QWidget* w, QLineEdit* line) {
    QString path = line->text().trimmed();
    QString newPath = QFileDialog::getExistingDirectory(w, "", path, // TODO text
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks); // TODO
    if (!newPath.isEmpty()) {
        line->setText(newPath);
    }
}

}
