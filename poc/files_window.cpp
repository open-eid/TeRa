/*
 * files_window.cpp
 *
 *  Created on: Nov 21, 2016
 */

#include <QDebug>

#include "files_window.h"

namespace ria_tera {

FileListWindow::FileListWindow(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
    setupUi(this);
    connect(rbList, SIGNAL(toggled(bool)), this, SLOT(viewTypeToggled(bool)));
    rbList->setChecked(true);
    rbList->setVisible(false);
    rbTree->setVisible(false);
    //rbTree->setChecked(true);

    model = new QStandardItemModel(); // TODO delete
    // http://doc.qt.io/qt-5/qtwidgets-itemviews-interview-model-h.html
    QStandardItem* item0 = new QStandardItem("0");
    item0->setCheckable(true);
    QStandardItem* item01 = new QStandardItem("01");
    item01->setCheckable(true);
    QStandardItem* item02 = new QStandardItem("02");
    item02->setCheckable(true);
    QStandardItem* item1 = new QStandardItem("1");
    item1->setCheckable(true);
    QStandardItem* item10 = new QStandardItem("10");
    item10->setCheckable(true);
    QStandardItem* item100 = new QStandardItem("100");
    item100->setCheckable(true);

    model->appendRow(item0);
    item0->appendRow(item01);
    item0->appendRow(item02);
    model->appendRow(item1);
    item1->appendRow(item10);
    item10->appendRow(item100);

    listView->setModel(model);
    treeView->setModel(model);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(btnSelect, SIGNAL(clicked()), this, SLOT(handleSelect()));
    connect(btnDeselect, SIGNAL(clicked()), this, SLOT(handleDeselect()));
    connect(btnSelectAll, SIGNAL(clicked()), this, SLOT(handleSelectAll()));
    connect(btnSelectNone, SIGNAL(clicked()), this, SLOT(handleSelectNone()));
}

FileListWindow::~FileListWindow() {
    delete model;
}

void FileListWindow::viewTypeToggled(bool listView) {
    if (listView) stackedWidget->setCurrentIndex(0);
    else stackedWidget->setCurrentIndex(1);
}

void FileListWindow::setFileList(QStringList const& files) {
    model->clear();
    for (int i = 0; i < files.size(); ++i) {
        QString path = files.at(i);
        QStandardItem* item = new QStandardItem(path);
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);
        model->appendRow(item);
    }
}

QStringList FileListWindow::extractSelectedFileList() {
    QStringList selectedFiles;

    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem* item = model->item(i);
        if (NULL == item) continue;
        if (Qt::Checked == item->checkState()) {
            QString filePath = item->text();
            selectedFiles.append(filePath);
        }
    }

    model->clear();
    return selectedFiles;
}

void FileListWindow::handleSelect() {
    setCheckStateForSelection(Qt::CheckState::Checked);
}

void FileListWindow::handleDeselect() {
    setCheckStateForSelection(Qt::CheckState::Unchecked);
}

void FileListWindow::handleSelectAll() {
    setCheckStateForAll(Qt::CheckState::Checked);
}

void FileListWindow::handleSelectNone() {
    setCheckStateForAll(Qt::CheckState::Unchecked);
}

void FileListWindow::setCheckStateForSelection(Qt::CheckState state) {
    QModelIndexList selection = listView->selectionModel()->selectedIndexes();
    for (auto it = selection.begin(); it != selection.end(); ++it) {
        QStandardItem* item = model->itemFromIndex(*it);
        item->setCheckState(state);
    }
}

void FileListWindow::setCheckStateForAll(Qt::CheckState state) {
    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem* item = model->item(i);
        item->setCheckState(state);
    }
}

}
