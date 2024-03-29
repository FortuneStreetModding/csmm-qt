#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QSettings>
#include "csmmmode.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    QSettings settings;
    switch (settings.value("csmmMode", INDETERMINATE).toInt()) {
    case EXPRESS:
        ui->quickSetup->setChecked(true);
        break;
    case ADVANCED:
        ui->advanced->setChecked(true);
        break;
    default:
        break;
    }
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::accept()
{
    QSettings settings;
    auto checkedBtn = ui->csmmMode->checkedButton();
    if (checkedBtn == ui->quickSetup) {
        settings.setValue("csmmMode", EXPRESS);
    } else if (checkedBtn == ui->advanced) {
        settings.setValue("csmmMode", ADVANCED);
    } else {
        settings.setValue("csmmMode", INDETERMINATE);
    }
    QDialog::accept();
}
