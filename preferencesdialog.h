#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();
private:
    Ui::PreferencesDialog *ui;
    void enableCacheSettings();
    void disableCacheSettings();
    void resetCacheDirectory();
private Q_SLOTS:
    void accept() override;
};

#endif // PREFERENCESDIALOG_H
