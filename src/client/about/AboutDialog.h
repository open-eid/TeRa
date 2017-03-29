#pragma once

#include <QDialog>

#include "ui_AboutDialog.h"

namespace ria_tera {

    class AboutDialog : public QDialog, public Ui::AboutDialog {
        Q_OBJECT

    public:
        explicit AboutDialog(QWidget *parent = 0);
        virtual ~AboutDialog();
    };

}

