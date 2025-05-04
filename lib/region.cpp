#include "region.h"
#include "qdir.h"
#include "QSettings"
#include "QTranslator"
#include "qwidget.h"

Region& Region::instance() {
    static Region instance;
    return instance;

    static QTranslator translator;
}

Region::Region(QObject *parent) : QObject(parent) {}

QStringList Region::availableProgramLanguages() const
{
    QDir dir(":/languages");
    QStringList translationFileNames = dir.entryList(QStringList("*.qm"), QDir::Files, QDir::Name);
    QStringList languagesAvailable;

    // then add the others before returning the list
    for (QString &fileName : translationFileNames){
        auto localeCode = QFileInfo(fileName).baseName();
        languagesAvailable.append(localeCode);
    }

    return languagesAvailable;
}

QStringList Region::availableGameReleaseTerritories(bool translated) const
{
    if(translated){
        return {
            tr("France"),
            tr("Germany"),
            tr("Italy"),
            tr("North America"),
            tr("Japan"),
            tr("Spain"),
            tr("United Kingdom")
        };
    }
    else{
        return {
            "France",
            "Germany",
            "Italy",
            "North America",
            "Japan",
            "Spain",
            "United Kingdom"
        };
    }
}

QString Region::currentSystemLanguage() const
{
    QLocale locale = QLocale();
    return locale.name();
}

QString Region::currentSystemTerritory() const
{
    QLocale locale = QLocale();
    return QLocale::territoryToString(locale.territory());
}

QString Region::getGameCodeFromTerritory(QString string)
{
    qInfo() << QString("getGameCodeFromTerritory string: %1").arg(string);

    QMap<QString, QString> codemap;

    codemap.insert("France","fr");
    codemap.insert("Germany","de");
    codemap.insert("Italy","it");
    codemap.insert("Japan","jp");
    codemap.insert("North America","en");
    codemap.insert("Spain","su");
    codemap.insert("United Kingdom","uk");

    if(codemap.contains(string)){
        qInfo() << QString("codemap contains the territory string");
        return codemap.value(string);
    }
    else {
        qInfo() << QString("codemap does not contain the territory string");
        return codemap.value(tr("North America"));
    }
}

QString Region::getTerritoryFromGameCode(QString string)
{
    QMap<QString, QString> codemap;
    codemap.insert("fr",tr("France"));
    codemap.insert("de",tr("Germany"));
    codemap.insert("it",tr("Italy"));
    codemap.insert("jp",tr("Japan"));
    codemap.insert("en",tr("North America"));
    codemap.insert("su",tr("Spain"));
    codemap.insert("uk",tr("United Kingdom"));

    if(codemap.contains(string)){
        return codemap.value(tr(string.toUtf8().constData()));
    }
    else {
        return codemap.value(tr("North America"));
    }
}

bool Region::applyProgramLanguage(QString string)
{
    qApp->removeTranslator(&translator);

    qInfo() << QString("Installing %1").arg(string);
    bool was_translation_successful = translator.load(QString(":/languages/%1").arg(string));

    if(was_translation_successful){
        qApp->installTranslator(&translator);
        qInfo() << QString("Translation was successful! %1 has been loaded.").arg(string);
    }
    else{
        qInfo() << "Translation was not successful.";
    }
    return was_translation_successful;
}

void Region::initializeRegionSettings()
{
    QSettings settings;

    if (!settings.contains("programLanguage")){
        // get the system language
        auto systemLanguageCode = currentSystemLanguage();
        auto languageCodesAvailable = availableProgramLanguages();

        if(languageCodesAvailable.contains(systemLanguageCode)){
            setProgramLanguage(systemLanguageCode);
        }
        else {
            // if we don't, set it to English and move on.
            setProgramLanguage("en_US");
        }
        applyProgramLanguage(settings.value("programLanguage").toString());
    }
    else {
        // if programLanguage is set, simply apply it.
        applyProgramLanguage(settings.value("programLanguage").toString());    }

    // territory settings
    if(!settings.contains("preferredGameTerritory")){
        auto systemTerritory = currentSystemTerritory();
        qInfo() << QString("Current System Territory: %1").arg(systemTerritory);
        auto gameReleaseTerritories = Region::instance().availableGameReleaseTerritories(true);

        if(gameReleaseTerritories.contains(systemTerritory)){
            // if our system's region matches, just use it!
            int index = gameReleaseTerritories.indexOf(systemTerritory);
            Region::instance().setPreferredGameTerritory(index);
        }
        else{
            // default back to North America
            Region::instance().setPreferredGameTerritory(3);
        }
    }
}

void Region::setProgramLanguage(QString string)
{
    QSettings settings;
    settings.setValue("programLanguage", string);
}

void Region::setPreferredGameTerritory(int index)
{
    QSettings settings;
    auto englishTerritoryString = availableGameReleaseTerritories(false).at(index);
    settings.setValue("preferredGameTerritory", englishTerritoryString);
}
