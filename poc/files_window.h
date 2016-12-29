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
    Q_OBJECT

public:
    explicit FileListWindow(QWidget *parent = 0);
    ~FileListWindow();

    void setFileList(QStringList const& files);
    QStringList extractSelectedFileList();
public slots:
    void handleSelectAll();
    void handleSelectNone();
private:
    void setCheckStateForAll(Qt::CheckState state);

    QStandardItemModel* model;
};

}

#endif /* FILES_WINDOW_H_ */
