#ifndef ADDITIONALMODSDIALOG_H
#define ADDITIONALMODSDIALOG_H

#include <QDialog>

namespace Ui {
class AdditionalModsDialog;
}

class AdditionalModsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AdditionalModsDialog(QWidget *parent = nullptr);
    ~AdditionalModsDialog();
    QVector<QString> GetModpackZips();

private:
    Ui::AdditionalModsDialog *ui;
};

#endif // ADDITIONALMODSDIALOG_H
