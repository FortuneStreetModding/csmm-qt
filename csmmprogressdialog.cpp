#include "csmmprogressdialog.h"
#include "lib/progresscanceled.h"


CSMMProgressDialog::CSMMProgressDialog(QWidget *parent, Qt::WindowFlags flags) : QProgressDialog(parent, flags) {
    setWindowFlag(Qt::WindowCloseButtonHint, false);
}

CSMMProgressDialog::CSMMProgressDialog(const QString &labelText, const QString &cancelButtonText, int minimum, int maximum, QWidget *parent, Qt::WindowFlags f, bool cancelable)
    : QProgressDialog(labelText, cancelButtonText, minimum, maximum, parent, f) {

    setWindowFlag(Qt::WindowCloseButtonHint, false);
    if (cancelable) setCancelButtonText("Cancel");
}

void CSMMProgressDialog::setValue(int progress)
{
    if (wasCanceled()) {
        throw ProgressCanceled("Canceled");
    }
    QProgressDialog::setValue(progress);
}

