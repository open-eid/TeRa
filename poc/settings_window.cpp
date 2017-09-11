/*
 * TeRa
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "settings_window.h"

#include <iostream>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include "gui_timestamper_processor.h"
#include "utils.h"

namespace ria_tera {

TeraSettingsWin::TeraSettingsWin(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
    setupUi(this);
    gotoInputDirTab();

    modelExclDir = new QStringListModel(); // TODO delete?
    listExclDir->setModel(modelExclDir);

    modelInclDir = new QStringListModel(); // TODO delete?
    listInclDir->setModel(modelInclDir);

    connect(btnAddExclDir, SIGNAL (clicked()), this, SLOT (handleAddExclDir()));
    connect(btnDelExclDir, SIGNAL (clicked()), this, SLOT (handleDelExclDir()));
    connect(btnExclDirSearch, SIGNAL (clicked()), this, SLOT (handleExclDirSearch()));

    connect(btnAddInclDir, SIGNAL (clicked()), this, SLOT (handleAddInclDir()));
    connect(btnDelInclDir, SIGNAL (clicked()), this, SLOT (handleDelInclDir()));
    connect(btnInclDirSearch, SIGNAL (clicked()), this, SLOT (handleInclDirSearch()));

    connect(cbStampDDoc, &QCheckBox::stateChanged, this, [this](int state){ stampTypeChanged(cbStampBDoc, state); });
    connect(cbStampBDoc, &QCheckBox::stateChanged, this, [this](int state){ stampTypeChanged(cbStampDDoc, state); });

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(handleTryAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

TeraSettingsWin::~TeraSettingsWin() {
    ;
}

void TeraSettingsWin::selectPage(TeraSettingsWin::PAGE openPage) {
    if (PAGE::INPUT_DIR == openPage) gotoInputDirTab();
}

void TeraSettingsWin::openInclDirSearch() {
    emit handleInclDirSearch();
}

void TeraSettingsWin::gotoInputDirTab() {
    tabWidget->setCurrentIndex(0);
}


void TeraSettingsWin::handleAddExclDir() {
    addDirToList(lineExclDir, modelExclDir);
}

void TeraSettingsWin::handleDelExclDir() {
    deleteSelected(listExclDir);
}

void TeraSettingsWin::handleExclDirSearch() {
    searchDir(this, lineExclDir, modelExclDir);
}

void TeraSettingsWin::handleAddInclDir() {
    addDirToList(lineInclDir, modelInclDir);
}

void TeraSettingsWin::handleDelInclDir() {
    deleteSelected(listInclDir);
}

void TeraSettingsWin::handleInclDirSearch() {
    searchDir(this, lineInclDir, modelInclDir);
}

void TeraSettingsWin::stampTypeChanged(QCheckBox* checkBox, int state) {
    // At least one of DDOC or BDOC must be selected
    if (state == Qt::Unchecked) {
        checkBox->setCheckState(Qt::Checked);
    }
}

void TeraSettingsWin::addDirToList(QString const& line, QStringListModel* model) {
    QString newPath = line.trimmed();
    if (newPath.isEmpty()) return;

    QDir dir(newPath);
    
    if (!dir.exists()) {
        QMessageBox::critical(this, tr("Error"), tr("Can't add '%1'. Directory does not exist.").arg(newPath));
        return;
    }

    int row = 0;
    while (row < model->rowCount()) {
        QModelIndex index = model->index(row);
        QString data = model->data(index, Qt::DisplayRole).toString();
        if (data == newPath) return;
        if (data > newPath) break;
        ++row;
    }
    model->insertRow(row);
    QModelIndex index = model->index(row);
    model->setData(index, newPath);
}

void TeraSettingsWin::addDirToList(QLineEdit* line, QStringListModel* model) {
    addDirToList(line->text(), model);
}

void TeraSettingsWin::deleteSelected(QListView* lv) {
    QModelIndexList indexes = lv->selectionModel()->selectedIndexes();
    qSort(indexes);
    for (int i = indexes.count()-1; i >= 0; --i) {
        lv->model()->removeRow(indexes.at(i).row());
    }
}

void TeraSettingsWin::searchDir(QWidget* w, QLineEdit* line, QStringListModel* model) {
    QString path = line->text().trimmed();
    QString newPath = QFileDialog::getExistingDirectory(w, "", path, // TODO text
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks); // TODO
    if (!newPath.isEmpty()) {
        line->setText(newPath);
        addDirToList(newPath, model);
    }
}

void TeraSettingsWin::handleTryAccept() {
    if (GuiTimestamperProcessor::checkInDirListWithMessagebox(this, *modelInclDir)) {
        accept();
    } else {
        gotoInputDirTab();
    }
}

}
