#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "qcombobox.h"
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
    void resetTemporaryDirectory();
    void buildPaletteMenu();
    void paletteActionTriggered();
    QString returnPaletteNameInCurrentLanguage(QString currentLocaleCode, QString englishPaletteName);
    void setPaletteLabel();
    void rebuildTerritoryComboBox();
    void rebuildLanguageComboBox(); 
private Q_SLOTS:
    void accept() override;
    void usePaletteHighlightColorCheckboxStatusChanged(int status);
protected:
    void changeEvent(QEvent *event) override;
Q_SIGNALS:
    void territoryChangedSignal(QString value);
};



#endif // PREFERENCESDIALOG_H
