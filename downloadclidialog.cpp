#include "downloadclidialog.h"
#include "ui_downloadclidialog.h"

DownloadCLIDialog::DownloadCLIDialog(const QString &defaultWitURL, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DownloadCLIDialog)
{
    ui->setupUi(this);
    ui->witURL->setText(defaultWitURL);
}

DownloadCLIDialog::~DownloadCLIDialog()
{
    delete ui;
}

QString DownloadCLIDialog::getWitURL() {
    return ui->witURL->text();
}
