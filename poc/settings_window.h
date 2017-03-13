/*
 * settings_window.h
 *
 *  Created on: Nov 18, 2016
 */

#ifndef SETTINGS_WINDOW_H_
#define SETTINGS_WINDOW_H_

#include <QDialog>
#include <QStringListModel>

#include "ui_SettingsDialog.h"

namespace ria_tera {

class TeraSettingsWin: public QDialog, public Ui::SettingsDialog
{
    Q_OBJECT

public:
    enum PAGE {__NONE, INPUT_DIR};

    explicit TeraSettingsWin(QWidget *parent = 0);
    ~TeraSettingsWin();

    QStringListModel* modelExclDir;
    QStringListModel* modelInclDir;

    void selectPage(TeraSettingsWin::PAGE openPage);
private slots:
    void handleTryAccept();

    void handleAddExclDir();
    void handleDelExclDir();
    void handleExclDirSearch();

    void handleAddInclDir();
    void handleDelInclDir();
    void handleInclDirSearch();
private:
    void gotoInputDirTab();
    void addDirToList(QString const& line, QStringListModel* model);
    void addDirToList(QLineEdit* line, QStringListModel* model);
    static void deleteSelected(QListView* lv);
    void searchDir(QWidget* w, QLineEdit* line, QStringListModel* model);
};

}

#endif /* SETTINGS_WINDOW_H_ */
