/*
 * files_window.cpp
 *
 *  Created on: Nov 21, 2016
 */
// TODO
// root node
// column title
// checkboxes TreeView
// DONE join list and tree
// coloring for tree http://stackoverflow.com/questions/20247065/qt-qtreeview-with-different-colors-for-subgroups-of-the-qtreeview-items
// Drive icon

#include <QDebug>

#include "files_window.h"

namespace ria_tera {

FileListWindow::FileListWindow(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
    setupUi(this);
    connect(rbList, SIGNAL(toggled(bool)), this, SLOT(viewTypeToggled(bool)));
    rbTree->setChecked(true);
    viewTypeToggled(false);

//// http://doc.qt.io/qt-5/qtwidgets-itemviews-interview-model-h.html
//QStandardItem* item0 = new QStandardItem("0");
//item0->setCheckable(true);
//QStandardItem* item01 = new QStandardItem("01");
//item01->setCheckable(true);
//QStandardItem* item02 = new QStandardItem("02");
//item02->setCheckable(true);
//QStandardItem* item1 = new QStandardItem("1");
//item1->setCheckable(true);
//QStandardItem* item10 = new QStandardItem("10");
//item10->setCheckable(true);
//QStandardItem* item100 = new QStandardItem("100");
//item100->setCheckable(true);
//
//model->appendRow(item0);
//item0->appendRow(item01);
//item0->appendRow(item02);
//model->appendRow(item1);
//item1->appendRow(item10);
//item10->appendRow(item100);

    listView->setModel(model.getModelListPtr());
    treeView->setModel(model.getModelTreePtr());

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(btnSelect, SIGNAL(clicked()), this, SLOT(handleSelect()));
    connect(btnDeselect, SIGNAL(clicked()), this, SLOT(handleDeselect()));
    connect(btnSelectAll, SIGNAL(clicked()), this, SLOT(handleSelectAll()));
    connect(btnSelectNone, SIGNAL(clicked()), this, SLOT(handleSelectNone()));
}

void FileListWindow::viewTypeToggled(bool listView) {
    if (listView) {
        stackedWidget->setCurrentIndex(0);
    } else {
        stackedWidget->setCurrentIndex(1);
    }

    btnSelect->setVisible(listView);
    btnDeselect->setVisible(listView);
}

void FileListWindow::setFileList(QStringList const& files) {
    model.setFileList(files);
    treeView->expandAll();
}

QStringList FileListWindow::extractSelectedFileList() {
    QStringList selectedFiles;

    for (int i = 0; i < model.getModelListPtr()->rowCount(); ++i) {
        QStandardItem* item = model.getModelListPtr()->item(i);
        if (NULL == item) continue;
        if (Qt::Checked == item->checkState()) {
            QString filePath = item->text();
            selectedFiles.append(filePath);
        }
    }

    model.clear();
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
        QStandardItem* item = model.getModelListPtr()->itemFromIndex(*it);
        item->setCheckState(state);
    }
}

void FileListWindow::setCheckStateForAll(Qt::CheckState state) {
    for (int i = 0; i < model.getModelListPtr()->rowCount(); ++i) {
        QStandardItem* item = model.getModelListPtr()->item(i);
        item->setCheckState(state);
    }
}

/////////////////////////////////////////////////////////////////////

JointModelItem::JointModelItem(const QString & text) : QStandardItem(text) {
    init();
}

JointModelItem::JointModelItem(const QIcon & icon, const QString & text) : QStandardItem(icon, text) {
    init();
}

void JointModelItem::init() {
    cntUnchecked = 0;
    cntPartChecked = 0;
    cntChecked = 0;
    parallelItem = NULL;
    parentItem = NULL;
}

void JointModelItem::setParallelItem(JointModelItem* pi) {
    if (pi == parallelItem) return;
    parallelItem = pi;
    parallelItem->setParallelItem(this);
}

Qt::CheckState JointModelItem::aggregatedCheckState() {
    if (cntChecked == 0 && cntPartChecked == 0) return Qt::CheckState::Unchecked;
    if (cntChecked > 0 && cntPartChecked == 0 && cntUnchecked == 0) return Qt::CheckState::Checked;
    return Qt::CheckState::PartiallyChecked;
}

void JointModelItem::changeCnt(int dx, Qt::CheckState state) {
    if (Qt::CheckState::Checked == state) cntChecked += dx;
    else if (Qt::CheckState::Unchecked == state) cntUnchecked += dx;
    else if (Qt::CheckState::PartiallyChecked == state) cntPartChecked += dx;
}

void JointModelItem::childChanged(Qt::CheckState stateOld, Qt::CheckState stateNew) {
    Qt::CheckState const state = checkState();

    changeCnt(-1, stateOld);
    changeCnt(1, stateNew);

    Qt::CheckState const newState = aggregatedCheckState();
    if (state != newState) {
        setCheckState(newState);
    }
}


void JointModelItem::setParentItem(JointModelItem* pi) {
    parentItem = pi;
    pi->childItems.append(this);

    pi->changeCnt(1, Qt::CheckState::Unchecked);
    pi->childChanged(Qt::CheckState::Unchecked, checkState());
}

void JointModelItem::setData(const QVariant &value, int role) {
    if (Qt::CheckStateRole == role) {
        Qt::CheckState const oldState = checkState();
        Qt::CheckState const newState = Qt::CheckState(qvariant_cast<int>(value));
        QStandardItem::setData(value, role);

        if (newState != oldState) {
            if (parallelItem) parallelItem->setCheckState(newState);
            if (parentItem) parentItem->childChanged(oldState, newState);
            if (Qt::CheckState::Checked == newState || Qt::CheckState::Unchecked == newState) {
                for (auto it = childItems.begin(); it != childItems.end(); ++it)
                    (*it)->setCheckState(newState);
            }
        }
    } else {
        QStandardItem::setData(value, role);
    }
}

JointModel::JointModel() {
    iconComputer = ip.icon(QFileIconProvider::IconType::Computer);
    iconFolder = ip.icon(QFileIconProvider::IconType::Folder);
    iconDrive = ip.icon(QFileIconProvider::IconType::Drive);
}

void JointModel::setFileList(QStringList const& f) {
    clear();
    files = f;
    files.sort();
    treeItems.clear();

    for (auto it = files.begin(); it != files.end(); ++it) {
        addFile(*it);
    }

    // treeItems contains references to strings in "files"
    treeItems.clear();
    files.clear();
}

void JointModel::addFile(QString const& path) {
    JointModelItem* itemL = addListItem(path);
    JointModelItem* itemT = addTreeItem(&path, true);
    itemL->setParallelItem(itemT);
}

JointModelItem* JointModel::addListItem(QString const& path) {
    JointModelItem* itemL = new JointModelItem(path);
    itemL->setCheckable(true);
    itemL->setCheckState(Qt::Checked);
    modelList.appendRow(itemL);
    return itemL;
}

JointModelItem* JointModel::addTreeItem(QStringRef const& path, bool file) {
    auto it = treeItems.find(path);
    if (it != treeItems.end()) return it.value();

    // create new item...
    // root node/whole compoter node
    if (path.isEmpty()) {
        // TODO name
        JointModelItem* item = new JointModelItem(iconComputer, "/");
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);
        modelTree.appendRow(item);
        treeItems.insert(path, item);
        return item;
    }

    // normal directories/files
    int separatorPos = path.lastIndexOf('/');
    QStringRef name = path.mid(separatorPos+1);
    QStringRef parentName = path.left((separatorPos >= 0 ? separatorPos : 0));
    JointModelItem* parentNode = addTreeItem(parentName, false);

    JointModelItem* item;
    if (file) {
        if (iconFile.isNull()) {
            iconFile = ip.icon(QFileInfo(path.toString()));
        }
        item = new JointModelItem(iconFile, name.toString());
    } else if (parentName.isEmpty()) {
        item = new JointModelItem(iconFolder, name.toString());
    }
    else {
        item = new JointModelItem(iconFolder, name.toString());
    }
    item->setCheckable(true);
    item->setCheckState(Qt::Checked);
    parentNode->appendRow(item);
    treeItems.insert(path, item);
    item->setParentItem(parentNode);

    return item;
}

}
