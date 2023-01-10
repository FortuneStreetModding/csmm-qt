#ifndef QUICKSETUPDIALOG_H
#define QUICKSETUPDIALOG_H

#include <QThreadPool>
#include <QDialog>
#include <QSharedPointer>
#include <QTemporaryDir>

namespace Ui {
class QuickSetupDialog;
}

class QuickSetupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QuickSetupDialog(const QString &defaultMarkerCode, bool defaultSeparateSaveGame, QWidget *parent = nullptr);
    ~QuickSetupDialog();
private:
    void updateRiivolutionEnabled();
    bool shouldPatchRiivolution();
    Ui::QuickSetupDialog *ui;
    QString defaultMarkerCode;
    bool defaultSeparateSaveGame;
    // QDialog interface
public Q_SLOTS:
    void accept() override;
};

#endif // QUICKSETUPDIALOG_H
