#ifndef DOWNLOADCLIDIALOG_H
#define DOWNLOADCLIDIALOG_H

#include <QDialog>

namespace Ui {
class DownloadCLIDialog;
}

class DownloadCLIDialog : public QDialog
{
    Q_OBJECT

public:
    DownloadCLIDialog(const QString &defaultWitURL, const QString &defaultWszstURL, QWidget *parent = nullptr);
    ~DownloadCLIDialog();
    QString getWitURL();
    QString getWszstURL();
private:
    Ui::DownloadCLIDialog *ui;
};

#endif // DOWNLOADCLIDIALOG_H
