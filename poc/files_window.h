/*
 * files_window.h
 *
 *  Created on: Nov 21, 2016
 */

#ifndef FILES_WINDOW_H_
#define FILES_WINDOW_H_

#include <QDialog>
#include <QStandardItemModel>

#include "ui_FileListDialog.h"

namespace ria_tera {

class FileListWindow: public QDialog, public Ui::FileListDialog {
public:
    explicit FileListWindow(QWidget *parent = 0);
    ~FileListWindow();
public:
    QStandardItemModel* model;
};

}

#endif /* FILES_WINDOW_H_ */
