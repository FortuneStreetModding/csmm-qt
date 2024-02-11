#include "preferencesdialog.h"
#include "lib/csmmnetworkmanager.h"
#include "qdir.h"
#include "qstandardpaths.h"
#include "ui_preferencesdialog.h"
#include <QFileDialog>
#include <QSettings>
#include "csmmmode.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    QSettings settings;
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
