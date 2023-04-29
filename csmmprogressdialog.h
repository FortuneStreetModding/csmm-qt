#ifndef CSMMPROGRESSDIALOG_H
#define CSMMPROGRESSDIALOG_H

#include <QProgressDialog>
#include <QObject>
#include <QException>

class CSMMProgressDialog : public QProgressDialog {
public:
    CSMMProgressDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    CSMMProgressDialog(const QString &labelText, const QString &cancelButtonText, int minimum, int maximum, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags(), bool cancelable = false);
    void setValue(int progress);
};

#endif // CSMMPROGRESSDIALOG_H
