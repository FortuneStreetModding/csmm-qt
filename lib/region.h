#ifndef REGION_H
#define REGION_H

#include "QApplication"
#include "qtranslator.h"

class Region : public QObject {
    Q_OBJECT

public:
    static Region& instance();
    QStringList availableProgramLanguages() const;
    QStringList availableGameReleaseTerritories(bool translated) const;
    QString currentSystemLanguage() const;
    QString currentSystemTerritory() const;

    QString getGameCodeFromTerritory(QString string);
    QString getTerritoryFromGameCode(QString string);

    QString getFlagEmoji(const QString &countryCode);
    QLocale getLocaleFromLanguageName(QString language);

    bool applyProgramLanguage(const QString string);
    void initializeRegionSettings();
    void setProgramLanguage(const QString string);
    void setPreferredGameTerritory(const int index);

private:
    explicit Region(QObject *parent = nullptr);
    QTranslator translator;
};

#endif // REGION_H
