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
    void openInclDirSearch();
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
