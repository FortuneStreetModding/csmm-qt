#ifndef QUICKSETUPDIALOG_H
#define QUICKSETUPDIALOG_H

#include <QThreadPool>
#include <QDialog>
#include <QSharedPointer>
#include <QTemporaryDir>

namespace Ui {
class QuickSetupDialog;
}

namespace csmm_quicksetup_detail {
class ProcessingGuard;
}

class QuickSetupDialog : public QDialog
{
    Q_OBJECT
    friend class csmm_quicksetup_detail::ProcessingGuard;
public:
    explicit QuickSetupDialog(const QString &defaultSaveId, QWidget *parent = nullptr);
    ~QuickSetupDialog();
private:
    Ui::QuickSetupDialog *ui;
    QString defaultSaveId;
    bool processing = false;
    // QDialog interface
public Q_SLOTS:
    void accept() override;
};

#endif // QUICKSETUPDIALOG_H
