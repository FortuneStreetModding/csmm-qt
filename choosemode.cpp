#include "choosemode.h"
#include "mainwindow.h"
#include "quicksetupdialog.h"
#include "ui_choosemode.h"

ChooseMode::ChooseMode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseMode)
{
    ui->setupUi(this);
    connect(ui->expressMode, &QPushButton::clicked, this, [this](bool) {
        auto dialog = new QuickSetupDialog("02", false);
        dialog->show();
        accept();
    });
    connect(ui->advancedMode, &QPushButton::clicked, this, [this](bool) {
        auto window = new MainWindow;
        window->show();
        accept();
    });
}

ChooseMode::~ChooseMode()
{
    delete ui;
}
