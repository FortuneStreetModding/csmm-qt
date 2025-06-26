#include "preferencesdialog.h"
#include "lib/csmmnetworkmanager.h"
#include "qdir.h"
#include "qstandardpaths.h"
#include "qtranslator.h"
#include "ui_preferencesdialog.h"
#include <QFileDialog>
#include <QSettings>
#include "csmmmode.h"

#include <QDir>
#include "ui_preferencesdialog.h"
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QStyleFactory>
#include "usersettings.h"
#include "QTemporaryDir"
#include "lib/region.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    this->resize(100,100);
    setWindowTitle(tr("Preferences"));

    ui->windowPaletteToolButton->setMenu(new QMenu);
    connect(ui->windowPaletteToolButton, &QToolButton::clicked, this, &PreferencesDialog::buildPaletteMenu);

    QSettings settings;
    setPaletteLabel();

    switch (settings.value("csmmMode", INDETERMINATE).toInt()) {
    case EXPRESS:
        ui->csmmModeQuickSetupButton->setChecked(true);
        break;
    case ADVANCED:
        ui->csmmModeAdvancedButton->setChecked(true);
        break;
    default:
        break;
    }

    bool useHighlightColorSetting = settings.value("window_palette/use_highlight_colors", 1).toBool();
    ui->usePaletteHighlightColorCheckbox->setChecked(useHighlightColorSetting);

    connect(ui->usePaletteHighlightColorCheckbox, &QCheckBox::checkStateChanged, this, [this](bool value){
        usePaletteHighlightColorCheckboxStatusChanged(value);
    });

    switch (settings.value("networkCacheMode").toInt()) {
    case 0:
    case 1:
        ui->enableCacheButton->setChecked(true);
        enableCacheSettings();
        break;
    case 2:
        ui->disableCacheButton->setChecked(true);
        disableCacheSettings();
        break;
    default:
        break;
    }

    auto clearCacheOnError = settings.value("networkAutoClearCacheOnError").toBool();
    ui->autoClearCacheCheckBox->setChecked(clearCacheOnError);

    auto size = settings.value("networkCacheSize").toInt();
    if (size >= 1 && size <= 10) {
        ui->cacheSizeSlider->setValue(size);
        ui->cacheSizeLabel->setText(QString::number(size) + "GB");
    } else {
        ui->cacheSizeSlider->setValue(4);
        ui->cacheSizeLabel->setText(QString::number(4) + "GB");
    }

    auto networkCacheDirName = settings.value("networkCacheDirectory").toString();
    if (!networkCacheDirName.isEmpty()) {
        ui->cacheDirectoryLabel->setText(networkCacheDirName);
    } else {
        resetCacheDirectory();
    }

    auto temporaryDirName = settings.value("temporaryDirectory").toString();
    if (!temporaryDirName.isEmpty()) {
        ui->temporaryDirectoryLabel->setText(temporaryDirName);
    } else {
        resetTemporaryDirectory();
    }

    QString chosenTerritory = settings.value("preferredGameTerritory","North America").toString();
    qInfo() << chosenTerritory + " ( chosen territory ) ";

    QString localeCode = settings.value("programLanguage","").toString();
    QLocale locale(localeCode);

    rebuildLanguageComboBox();

    int currentLanguageIndex = ui->languageComboBox->findText(locale.nativeLanguageName());
    if(currentLanguageIndex == -1){
        // if we didn't find it, we're likely set to English -- see the comments in the rebuildLanguageComboBox()
        // function for more details as to why this is necessary.
        currentLanguageIndex = ui->languageComboBox->findText(QLocale::languageToString(locale.language()));
    }
    if (currentLanguageIndex != -1) {
        ui->languageComboBox->setCurrentIndex(currentLanguageIndex);  // Select the item if found
    }

    QStringList currentTerritories = Region::instance().availableGameReleaseTerritories(false);
    QStringList translated = Region::instance().availableGameReleaseTerritories(true);

    QString chosenTerritoryIndex = QString::number(currentTerritories.indexOf(chosenTerritory));
    qInfo() << currentTerritories.join(" ") + " ( current territories ) "; // in english
    qInfo() << translated.join(" ") + " ( current territories - translated ) "; // in the selected language

    qInfo() << chosenTerritoryIndex + " ( chosenTerritoryIndex ) ";

    rebuildTerritoryComboBox();

    if (chosenTerritory.toInt() != -1) {
        ui->preferredGameRegionComboBox->setCurrentIndex(chosenTerritoryIndex.toInt());  // Select the item if found
    }

    connect(ui->updateCacheDirectoryButton, &QPushButton::clicked, this, [this](bool){
        auto networkCacheDirNname = QFileDialog::getExistingDirectory(this, tr("Set CSMM Network Cache Directory"), nullptr);
        if (!networkCacheDirNname.isEmpty()) {
            ui->cacheDirectoryLabel->setText(networkCacheDirNname);
        }
    });

    connect(ui->updateTemporaryDirectoryButton, &QPushButton::clicked, this, [this](bool){
        auto temporaryDirName = QFileDialog::getExistingDirectory(this, tr("Set CSMM Temporary Directory"), nullptr);
        if (!temporaryDirName.isEmpty()) {
            ui->temporaryDirectoryLabel->setText(temporaryDirName);
        }
    });

    connect(ui->resetCacheDirectoryButton, &QPushButton::clicked, this, [this](bool){
        resetCacheDirectory();
    });

    connect(ui->resetTemporaryDirectoryButton, &QPushButton::clicked, this, [this](bool){
        resetTemporaryDirectory();
    });

    connect(ui->clearNetworkCacheButton, &QPushButton::clicked, this, [this](bool){
        CSMMNetworkManager::clearNetworkCache();
    });

    connect(ui->enableCacheButton, &QPushButton::clicked, this, [this](bool){
        enableCacheSettings();
    });

    connect(ui->disableCacheButton, &QPushButton::clicked, this, [this](bool){
        disableCacheSettings();
    });

    connect(ui->cacheSizeSlider, &QSlider::sliderMoved, this, [this](int position){
        auto size_str = QString::number(position);
        ui->cacheSizeLabel->setText(size_str + "GB");
    });

    connect(ui->languageComboBox, &QComboBox::currentTextChanged, this, [this](QString text){
        auto selectedLanguage = ui->languageComboBox->currentData().toString(); // this should be the locale code
        qInfo() << "saving locale code to settings: " + selectedLanguage;
        Region::instance().setProgramLanguage(selectedLanguage);
        Region::instance().applyProgramLanguage(selectedLanguage);

        QLocale locale(selectedLanguage);

        qInfo() << QString("language changed to %1").arg(QLocale::languageToString(locale.language()));
    });

    connect(ui->preferredGameRegionComboBox, &QComboBox::currentIndexChanged, this, [this](int index){
        if(index != -1){
            auto territories = Region::instance().availableGameReleaseTerritories(true);
            auto territoryString = territories.at(index);
            qInfo() << QString("territory changed to %1").arg(territoryString);
            auto index = ui->preferredGameRegionComboBox->currentIndex();
            Region::instance().setPreferredGameTerritory(index);
            territoryChangedSignal(Region::instance().availableGameReleaseTerritories(false).at(index));
        }
        else{
            qInfo() << "-1 received by QComboBox::currentIndexChanged";
        }
    });
}

void PreferencesDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        qInfo() << "A PreferencesDialog::changeEvent() has fired!";

        rebuildTerritoryComboBox();
        setPaletteLabel();
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

void PreferencesDialog::rebuildTerritoryComboBox()
{
    auto index = ui->preferredGameRegionComboBox->currentIndex();
    auto territories  = Region::instance().availableGameReleaseTerritories(false);
    QStringList translated = Region::instance().availableGameReleaseTerritories(true);
    // blocking signals here so we don't trigger a region change event
    // by changing the current index of the combo box.
    ui->preferredGameRegionComboBox->blockSignals(true);
    ui->preferredGameRegionComboBox->clear();
    ui->preferredGameRegionComboBox->addItems(translated);
    ui->preferredGameRegionComboBox->setCurrentIndex(index);
    ui->preferredGameRegionComboBox->blockSignals(false);
}

void PreferencesDialog::rebuildLanguageComboBox(){
    QComboBox* combobox = ui->languageComboBox;

    auto index = combobox->currentIndex();
    auto languages = Region::instance().availableProgramLanguages();

    qInfo() << languages;

    combobox->blockSignals(true);
    combobox->clear();

    for(auto &l: languages){
        QLocale locale(l);

        qInfo() << "Native: " + locale.nativeLanguageName();
        qInfo() << "Generic: " + QLocale::languageToString(locale.language());
        qInfo() << "l: " + l;

        // if English, we want to populate the comboBox with only "English", rather
        // than regional variants like "American English" or "British English", because
        // for the sake of our program, there is no difference between regions.

        if(l == "en_US"){
            combobox->addItem(QLocale::languageToString(locale.language()), l);
        }
        else {
            // for all other languages, we want to simply populate with the native
            // language name.
            combobox->addItem(locale.nativeLanguageName(), l);
        }
    }

    combobox->setCurrentIndex(index);
    combobox->blockSignals(false);
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::accept()
{
    QSettings settings;
    // default csmm mode
    auto csmmModeBtn = ui->csmmModeBtnGroup->checkedButton();
    if (csmmModeBtn == ui->csmmModeQuickSetupButton) {
        settings.setValue("csmmMode", EXPRESS);
    } else if (csmmModeBtn == ui->csmmModeAdvancedButton) {
        settings.setValue("csmmMode", ADVANCED);
    } else {
        settings.setValue("csmmMode", INDETERMINATE);
    }

    // network cache enable/disable
    auto networkCacheBtn = ui->cacheModeBtnGroup->checkedButton();
    if (networkCacheBtn == ui->enableCacheButton) {
        settings.setValue("networkCacheMode", 1);
    } else if (networkCacheBtn == ui->disableCacheButton) {
        settings.setValue("networkCacheMode", 2);
    } else { // enable if not set
        settings.setValue("networkCacheMode", 1);
    }

    auto clearCacheOnError = ui->autoClearCacheCheckBox->isChecked();
    settings.setValue("networkAutoClearCacheOnError", clearCacheOnError);

    auto networkCacheSize = ui->cacheSizeSlider->value();
    settings.setValue("networkCacheSize", networkCacheSize);

    auto networkCacheDirectory = ui->cacheDirectoryLabel->text();
    settings.setValue("networkCacheDirectory", networkCacheDirectory);

    auto temporaryCacheDirectory = ui->temporaryDirectoryLabel->text();
    settings.setValue("temporaryDirectory", temporaryCacheDirectory);

    QDialog::accept();
}

void PreferencesDialog::enableCacheSettings()
{
    ui->cacheDirectoryGroupBox->setEnabled(true);
    ui->maximumCacheSizeGroupBox->setEnabled(true);
}


void PreferencesDialog::disableCacheSettings()
{
    ui->cacheDirectoryGroupBox->setDisabled(true);
    ui->maximumCacheSizeGroupBox->setDisabled(true);
}


void PreferencesDialog::resetCacheDirectory()
{
    QDir applicationCacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    auto defaultNetworkCacheDir = applicationCacheDir.filePath("networkCache");
    ui->cacheDirectoryLabel->setText(defaultNetworkCacheDir);
}

void PreferencesDialog::resetTemporaryDirectory()
{
    QTemporaryDir d;
    QSettings settings;
    settings.setValue("temporaryDirectory", d.path());
    ui->temporaryDirectoryLabel->setText(d.path());
    d.remove();
}

// Window Palette

// The key is the palette name, and the value is a QJsonObject representing the colors of the palette
QMap<QString, QJsonObject> palette_files;

void PreferencesDialog::buildPaletteMenu()
{
    ui->windowPaletteToolButton->menu()->clear();

    // get the list of JSON palette files
    QString palettePath = ":/palettes/";
    QDir paletteDir = palettePath;
    QStringList paletteFiles = paletteDir.entryList(QStringList() << "*.json", QDir::Files);

    // having references to the categories encountered and submenus created
    // will be helpful when adding palettes to those category submenus
    QStringList categories;
    QMap<QString, QMenu*> submenus;

    // iterate over each JSON file
    for (const QString& jsonFile : paletteFiles) {
        QJsonDocument doc = readJsonFile(palettePath, jsonFile);

        // grab the name and category and whatever other data from these palettes
        QJsonObject rootObj = doc.object();
        //QString name = rootObj.value("name").toString();
        //QString category = rootObj.value("category").toString();


        QSettings settings;
        QString currentLanguageLocaleCode = settings.value("programLanguage","").toString();

        // creating the variables first, just in case
        QString name = "not set";
        QString englishName = "not set";
        QString category = "not set";

        if(rootObj.contains("name") && rootObj["name"].isObject()){
            // just being extra careful that the json is in the proper format
            QJsonObject nameObj = rootObj["name"].toObject();

            if(nameObj.contains(currentLanguageLocaleCode)){
                name = nameObj.value(currentLanguageLocaleCode).toString();
                englishName = nameObj.value("en_US").toString();
            }
            else{
                name = nameObj.value("en_US").toString();
                englishName = name;
            }
        }

        if(rootObj.contains("category") && rootObj["category"].isObject()){
            // just being extra careful that the json is in the proper format
            QJsonObject categoryObj = rootObj["category"].toObject();

            if(categoryObj.contains(currentLanguageLocaleCode)){
                category = categoryObj.value(currentLanguageLocaleCode).toString();
            }
            else{
                category = categoryObj.value("en_US").toString();
            }
        }

        // this is what we'll pass to setChosenPalette
        QJsonObject colors = rootObj.value("colors").toObject();

        // ...so that function can use lines like this.
        // QString window = colors.value("window").toString();

        // build a submenu for the category if it does not already exist
        if(!categories.contains(category))
        {
            categories.append(category);
            submenus.insert(category, ui->windowPaletteToolButton->menu()->addMenu(category));
        }

        // add the palette as an action in the submenu of its category
        QAction *action = new QAction(name, this);
        // set the English name so we can set this value in Settings
        action->setData(englishName);

        connect(action, &QAction::triggered, this, &PreferencesDialog::paletteActionTriggered);
        submenus.value(category)->addAction(action);

        palette_files.insert(englishName, colors);
    }

    // finally show the menu after building it, so we don't require
    // them to click the tiny triangle on the side of the button
    ui->windowPaletteToolButton->showMenu();
}

void PreferencesDialog::paletteActionTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action) {
        QString paletteName = action->text();
        QString paletteEnglishName = action->data().toString();

        // set the window palette label with the name of the new palette
        ui->windowPaletteLabel->setText(paletteName);

        // check whether or not to use highlight colors
        bool useHighlightColors = ui->usePaletteHighlightColorCheckbox->isChecked();

        // apply the palette
        setChosenPalette(palette_files.value(paletteEnglishName), useHighlightColors);

        // set the palette as chosen in QSettings
        saveUserWindowPalette(paletteEnglishName, palette_files.value(paletteEnglishName), useHighlightColors);
    }
}

QString PreferencesDialog::returnPaletteNameInCurrentLanguage(QString currentLocaleCode, QString englishPaletteName){
    // get the list of JSON palette files
    QString palettePath = ":/palettes/";
    QDir paletteDir = palettePath;
    QStringList paletteFiles = paletteDir.entryList(QStringList() << "*.json", QDir::Files);

    // return the english palette name if we cannot otherwise find it;
    // if we can find it we'll overwrite it in the loop below.
    QString nameInCurrentLanguage = englishPaletteName;

    // iterate over each JSON file
    for (const QString& jsonFile : paletteFiles) {
        QJsonDocument doc = readJsonFile(palettePath, jsonFile);

        // grab the name and category and whatever other data from these palettes
        QJsonObject rootObj = doc.object();

        if(rootObj.contains("name") && rootObj["name"].isObject()){
            // just being extra careful that the json is in the proper format
            QJsonObject nameObj = rootObj["name"].toObject();

            if(englishPaletteName == nameObj.value("en_US").toString()){
                if(nameObj.contains(currentLocaleCode)){
                    nameInCurrentLanguage = nameObj.value(currentLocaleCode).toString();
                }
            }
        }
    }
    return nameInCurrentLanguage;
}

void PreferencesDialog::setPaletteLabel(){
    QSettings settings;
    QString localeCode = settings.value("programLanguage","").toString();
    QString englishPaletteName = settings.value("window_palette/name", "not set").toString();

    // if language is not english, set the palette name in the appropriate language
    if(localeCode == "en_us"){
        ui->windowPaletteLabel->setText(englishPaletteName);
    }
    else{
        ui->windowPaletteLabel->setText(returnPaletteNameInCurrentLanguage(localeCode, englishPaletteName));
    }
}

void PreferencesDialog::usePaletteHighlightColorCheckboxStatusChanged(int status)
{
    bool useHighlightColors = status;
    // if false, we're disabling the use of the palette's highlight color and highlight text color entries.
    // if true, we're enabling their use.
    QJsonObject palette = getSavedUserWindowPalette();
    setChosenPalette(palette, useHighlightColors);
    saveUserWindowPalette(palette.value("name").toString(), palette, useHighlightColors);
}

