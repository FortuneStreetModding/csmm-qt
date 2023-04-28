#ifndef CSMMPROGRESSDIALOG_H
#define CSMMPROGRESSDIALOG_H

#include <QProgressDialog>
#include <QObject>
#include <QException>

class CSMMProgressDialog : public QProgressDialog {
public:
    CSMMProgressDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    CSMMProgressDialog(const QString &labelText, const QString &cancelButtonText, int minimum, int maximum, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags(), bool cancelable = false);
    void checkCancel();
};

class ProgressCanceled : public QException, public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    const char *what() const noexcept override { return std::runtime_error::what(); }
    ProgressCanceled(const QString &str) : std::runtime_error(str.toStdString()) {}
    void raise() const override { throw *this; }
    ProgressCanceled *clone() const override { return new ProgressCanceled(*this); }
};

#endif // CSMMPROGRESSDIALOG_H
