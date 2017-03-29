#include "AboutDialog.h"

#include <QPushButton>

namespace ria_tera {

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    tabWidget->removeTab(1);

    QPushButton* closeButton = buttonBox->addButton(tr("Close"), QDialogButtonBox::AcceptRole); // (QMessageBox::Yes, tr("Yes"));
    connect(closeButton, SIGNAL(clicked(bool)), this, SLOT(accept()));

    QString package; // base software version
    version->setText(tr("%1 version %2, released %3%4")
        .arg(qApp->applicationName(), qApp->applicationVersion(), BUILD_DATE, package));
}

AboutDialog::~AboutDialog() {}

}
