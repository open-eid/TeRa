/*
 * files_window.cpp
 *
 *  Created on: Nov 21, 2016
 */

#include "files_window.h"

namespace ria_tera {

FileListWindow::FileListWindow(QWidget *parent) : QDialog(parent) {
    setupUi(this);

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
}

FileListWindow::~FileListWindow() {}


}
