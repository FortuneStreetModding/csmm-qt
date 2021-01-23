#include "validationerrordialog.h"
#include "ui_validationerrordialog.h"

ValidationErrorDialog::ValidationErrorDialog(const QStringList &errors, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ValidationErrorDialog)
{
    ui->setupUi(this);
    ui->listWidget->addItems(errors);
}

ValidationErrorDialog::~ValidationErrorDialog()
{
    delete ui;
}
