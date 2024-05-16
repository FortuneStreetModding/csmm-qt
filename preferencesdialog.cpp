#include "preferencesdialog.h"
#include "lib/csmmnetworkmanager.h"
#include "qdir.h"
#include "qstandardpaths.h"
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

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    this->resize(100,100);
    setWindowTitle("Preferences");

    ui->windowPaletteToolButton->setMenu(new QMenu);
    connect(ui->windowPaletteToolButton, &QToolButton::clicked, this, &PreferencesDialog::buildPaletteMenu);

    QSettings settings;
    ui->windowPaletteLabel->setText(settings.value("window_palette/name", "not set").toString());

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

    connect(ui->usePaletteHighlightColorCheckbox, &QCheckBox::stateChanged, this, [this](bool value){
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

    auto dirname = settings.value("networkCacheDirectory").toString();
    if (!dirname.isEmpty()) {
        ui->cacheDirectoryLabel->setText(dirname);
    } else {
        resetCacheDirectory();
    }

    connect(ui->updateCacheDirectoryButton, &QPushButton::clicked, this, [this](bool){
        auto dirname = QFileDialog::getExistingDirectory(this, "Set CSMM Network Cache Directory", nullptr);
        if (!dirname.isEmpty()) {
            ui->cacheDirectoryLabel->setText(dirname);
        }
    });

    connect(ui->resetCacheDirectoryButton, &QPushButton::clicked, this, [this](bool){
        resetCacheDirectory();
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
        QString name = rootObj.value("name").toString();
        QString category = rootObj.value("category").toString();

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
        connect(action, &QAction::triggered, this, &PreferencesDialog::paletteActionTriggered);
        submenus.value(category)->addAction(action);

        palette_files.insert(name, colors);
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

        // set the window palette label with the name of the new palette
        ui->windowPaletteLabel->setText(paletteName);

        // check whether or not to use highlight colors
        bool useHighlightColors = ui->usePaletteHighlightColorCheckbox->isChecked();

        // apply the palette
        setChosenPalette(palette_files.value(paletteName), useHighlightColors);

        // set the palette as chosen in QSettings
        saveUserWindowPalette(paletteName, palette_files.value(paletteName), useHighlightColors);
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

