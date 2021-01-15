#include "downloadclidialog.h"
#include "ui_downloadclidialog.h"

DownloadCLIDialog::DownloadCLIDialog(const QString &defaultWitURL, const QString &defaultWszstURL, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DownloadCLIDialog)
{
    ui->setupUi(this);
    ui->witURL->setText(defaultWitURL);
    ui->wszstURL->setText(defaultWszstURL);
}

DownloadCLIDialog::~DownloadCLIDialog()
{
    delete ui;
}

QString DownloadCLIDialog::getWitURL() {
    return ui->witURL->text();
}

QString DownloadCLIDialog::getWszstURL() {
    return ui->wszstURL->text();
}
