/*
 * files_window.h
 *
 *  Created on: Nov 21, 2016
 */

#ifndef FILES_WINDOW_H_
#define FILES_WINDOW_H_

#include <QDialog>
#include <QFileIconProvider>
#include <QStandardItemModel>

#include "ui_FileListDialog.h"

namespace ria_tera {

// TODO add count on selected number of files
class JointModelItem : public QStandardItem {
public:
    JointModelItem(const QString & text);
    JointModelItem(const QIcon & icon, const QString & text);
    void setParallelItem(JointModelItem* pi);
    void setParentItem(JointModelItem* pi);
    virtual void setData(const QVariant &value, int role = Qt::UserRole + 1);
private:
    void init();
    Qt::CheckState aggregatedCheckState();
    void changeCnt(int dx, Qt::CheckState state);
    void childChanged(Qt::CheckState stateOld, Qt::CheckState stateNew);

    int cntUnchecked;
    int cntPartChecked;
    int cntChecked;
    JointModelItem* parallelItem;
    JointModelItem* parentItem;
    QList<JointModelItem*> childItems;
};

/// Factory for synchronized models to show in QListView and QTreeView
class JointModel {
public:
    JointModel();
    void setFileList(QStringList const& files);
    QStandardItemModel* getModelListPtr() { return &modelList; };
    QStandardItemModel* getModelTreePtr() { return &modelTree; };
    void clear() { modelList.clear(); modelTree.clear(); };
private:
    void addFile(QString const& path);
    JointModelItem* addListItem(QString const& path);
    JointModelItem* addTreeItem(QStringRef const& path, bool file);

    QStringList files;
    QMap<QStringRef, JointModelItem*> treeItems;

    QFileIconProvider ip;
    QIcon iconComputer;
    QIcon iconDrive;
    QIcon iconFolder;
    QIcon iconFile;
    QStandardItemModel modelList;
    QStandardItemModel modelTree;
};

class FileListWindow: public QDialog, public Ui::FileListDialog {
    Q_OBJECT

public:
    explicit FileListWindow(QWidget *parent = 0);

    void setFileList(QStringList const& files);
    QStringList extractSelectedFileList();
public slots:
    void handleSelect();
    void handleDeselect();
    void handleSelectAll();
    void handleSelectNone();
private slots:
    void viewTypeToggled(bool listView);
private:
    void setCheckStateForSelection(Qt::CheckState state);
    void setCheckStateForAll(Qt::CheckState state);

    JointModel model;
};

}

#endif /* FILES_WINDOW_H_ */
