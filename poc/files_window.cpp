/*
 * files_window.cpp
 *
 *  Created on: Nov 21, 2016
 */

#include <QDebug>

#include "files_window.h"

namespace ria_tera {

FileListWindow::FileListWindow(QWidget *parent) : QDialog(parent) {
    setupUi(this);
    stackedWidget->setCurrentIndex(0);

    model = new QStandardItemModel(); // TODO delete
// http://doc.qt.io/qt-5/qtwidgets-itemviews-interview-model-h.html
//    QStandardItem* item0 = new QStandardItem("0");
//    item0->setCheckable(true);
//    QStandardItem* item01 = new QStandardItem("01");
//    QStandardItem* item02 = new QStandardItem("02");
//    QStandardItem* item1 = new QStandardItem("1");
//    //item1->setCheckable(true);
//    QStandardItem* item10 = new QStandardItem("10");
//    QStandardItem* item100 = new QStandardItem("100");
//
//    model->appendRow(item0);
//    item0->appendRow(item01);
//    item0->appendRow(item02);
//    model->appendRow(item1);
//    item1->appendRow(item10);
//    item10->appendRow(item100);

    listView->setModel(model);
    // TODO treeView->setModel(model);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(btnSelectAll, SIGNAL(clicked()), this, SLOT(handleSelectAll()));
    connect(btnSelectNone, SIGNAL(clicked()), this, SLOT(handleSelectNone()));
}

FileListWindow::~FileListWindow() {
    delete model;
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

void FileListWindow::handleSelectAll() {
    setCheckStateForAll(Qt::CheckState::Checked);
}

void FileListWindow::handleSelectNone() {
    setCheckStateForAll(Qt::CheckState::Unchecked);
}

void FileListWindow::setCheckStateForAll(Qt::CheckState state) {
    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem* item = model->item(i);
        item->setCheckState(state);
    }
}

}
