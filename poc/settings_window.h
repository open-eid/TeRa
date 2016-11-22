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
    explicit TeraSettingsWin(QWidget *parent = 0);
    ~TeraSettingsWin();

    QStringListModel* modelExclDir;
    QStringListModel* modelInclDir;
private slots:
    void handleAddExclDir();
    void handleDelExclDir();
    void handleExclDirSearch();

    void handleAddInclDir();
    void handleDelInclDir();
    void handleInclDirSearch();
private:
    static void addDirToList(QLineEdit* line, QStringListModel* model);
    static void deleteSelected(QListView* lv);
    static void searchDir(QWidget* w, QLineEdit* line);
};

}

#endif /* SETTINGS_WINDOW_H_ */
