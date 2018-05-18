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

#include "AboutDialog.h"

namespace ria_tera {

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    tabWidget->removeTab(1);

    buttonBox->addButton(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AboutDialog::reject);

    version->setText(tr("%1 version %2, released %3%4")
        .arg(qApp->applicationName(), qApp->applicationVersion(), BUILD_DATE, QString()));
    elLogo->setPixmap(QPixmap(tr(":/images/EL_Regionaalareng_horisontaal.png")));
}

}
