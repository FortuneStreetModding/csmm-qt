#ifndef VALIDATIONERRORDIALOG_H
#define VALIDATIONERRORDIALOG_H

#include <QDialog>

namespace Ui {
class ValidationErrorDialog;
}

class ValidationErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ValidationErrorDialog(const QStringList &errors, QWidget *parent = nullptr);
    ~ValidationErrorDialog();

private:
    Ui::ValidationErrorDialog *ui;
};

#endif // VALIDATIONERRORDIALOG_H
