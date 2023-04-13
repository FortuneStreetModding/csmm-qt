#include "csmmprogressdialog.h"


CSMMProgressDialog::CSMMProgressDialog(QWidget *parent, Qt::WindowFlags flags) : QProgressDialog(parent, flags) {
    setWindowFlag(Qt::WindowCloseButtonHint, false);
}

CSMMProgressDialog::CSMMProgressDialog(const QString &labelText, const QString &cancelButtonText, int minimum, int maximum, QWidget *parent, Qt::WindowFlags f)
    : QProgressDialog(labelText, cancelButtonText, minimum, maximum, parent, f) {

    setWindowFlag(Qt::WindowCloseButtonHint, false);
}
