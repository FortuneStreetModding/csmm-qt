#include "choosemode.h"
#include "mainwindow.h"
#include "quicksetupdialog.h"
#include "ui_choosemode.h"
#include "csmmmode.h"

ChooseMode::ChooseMode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseMode)
{
    ui->setupUi(this);
    connect(ui->expressMode, &QPushButton::clicked, this, [this](bool) {
        auto dialog = new QuickSetupDialog("02", false);
        dialog->show();
        if (ui->dontAskAgain->isChecked()) {
            settings.setValue("csmmMode", EXPRESS);
        }
        accept();
    });
    connect(ui->advancedMode, &QPushButton::clicked, this, [this](bool) {
        auto window = new MainWindow;
        window->show();
        if (ui->dontAskAgain->isChecked()) {
            settings.setValue("csmmMode", ADVANCED);
        }
        accept();
    });
}

ChooseMode::~ChooseMode()
{
    delete ui;
}
