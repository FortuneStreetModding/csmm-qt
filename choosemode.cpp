#include "choosemode.h"
#include "ui_choosemode.h"

ChooseMode::ChooseMode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseMode)
{
    ui->setupUi(this);
}

ChooseMode::~ChooseMode()
{
    delete ui;
}
