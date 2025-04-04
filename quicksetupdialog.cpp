#include "quicksetupdialog.h"
#include "lib/progresscanceled.h"
#include "preferencesdialog.h"
#include "ui_quicksetupdialog.h"

#include <QtConcurrent>
#include <QFileDialog>
#include <QMessageBox>
#include <filesystem>

#include "lib/await.h"
#include "lib/exewrapper.h"
#include "lib/mods/modloader.h"
#include "lib/mods/csmmmodpack.h"
#include "lib/configuration.h"
#include "lib/riivolution.h"
#include "csmmprogressdialog.h"
#include "mainwindow.h"

QuickSetupDialog::QuickSetupDialog(const QString &defaultMarkerCode, bool defaultSeparateSaveGame, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuickSetupDialog), defaultMarkerCode(defaultMarkerCode), defaultSeparateSaveGame(defaultSeparateSaveGame)
{
    ui->setupUi(this);
    this->resize(size().width(),100);
    ui->markerCode->setText(defaultMarkerCode);
    ui->separateSaveGame->setChecked(defaultSeparateSaveGame);

    connect(ui->chooseInputGameFolder, &QPushButton::clicked, this, [this](bool){
        auto dirname = QFileDialog::getExistingDirectory(this, "Open Fortune Street Directory", nullptr);
        if (!dirname.isEmpty()) {
            ui->inputGameLoc->setText(dirname);
            updateButtonBoxEnabled();
        }
    });
    connect(ui->chooseInputWbfsIso, &QPushButton::clicked, this, [this](bool){
        auto isoWbfs = QFileDialog::getOpenFileName(this, "Import .wbfs or .iso file", QString(), "Fortune Street disc files (*.wbfs *.iso *.ciso)", nullptr);
        if (!isoWbfs.isEmpty()) {
            ui->inputGameLoc->setText(isoWbfs);
            updateButtonBoxEnabled();
        }
    });
    connect(ui->chooseModpackZip, &QPushButton::clicked, this, [this](bool){
        auto file = QFileDialog::getOpenFileName(this, "Import modpack", QString(), "Modpack zip file (*.zip)", nullptr);
        if (!file.isEmpty()) {
            ui->modpackZip->setText(file);
            updateButtonBoxEnabled();
        }
    });
    connect(ui->addModZip, &QPushButton::clicked, this, [this](bool) {
        auto file = QFileDialog::getOpenFileName(this, "Import additional modpack", QString(), "Modpack zip file (*.zip)", nullptr);
        if (!file.isEmpty()) {
            ui->additionalMods->addItem(file);
            updateButtonBoxEnabled();
        }
    });
    connect(ui->removeModZip, &QPushButton::clicked, this, [this](bool) {
        for (auto item: ui->additionalMods->selectedItems()) {
            delete item;
        }
    });


    exportToWbfsIso = new QPushButton("Export to File");
    exportToWbfsIso->setEnabled(false);
    exportToWbfsIso->setDefault(true);
    ui->buttonBox->addButton(exportToWbfsIso, QDialogButtonBox::AcceptRole);

    exportToExtractedFolder = new QPushButton("Export to Folder");
    exportToExtractedFolder->setEnabled(false);
    ui->buttonBox->addButton(exportToExtractedFolder, QDialogButtonBox::AcceptRole);

    toPreferences = new QPushButton("Preferences");
    toPreferences->setAutoDefault(false);
    ui->buttonBox->addButton(toPreferences, QDialogButtonBox::ResetRole);

    toAdvancedMode = new QPushButton("Switch to Advanced CSMM");
    toAdvancedMode->setAutoDefault(false);
    ui->buttonBox->addButton(toAdvancedMode, QDialogButtonBox::ResetRole);

    connect(ui->enableRiivolutionGroupBox, &QGroupBox::toggled, this, [this](bool checked) {
        updateButtonBoxEnabled();
    });

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &QuickSetupDialog::onResultClick);

    updateButtonBoxEnabled();

    setWindowTitle(QString("CSMM %1: Quick Setup").arg(CSMM_VERSION));
}

QuickSetupDialog::~QuickSetupDialog()
{
    delete ui;
}

bool QuickSetupDialog::shouldPatchRiivolution()
{
    return ui->enableRiivolutionGroupBox->isChecked();
}

namespace csmm_quicksetup_detail {
class ProcessingGuard : public QObject {
public:
    ProcessingGuard(QuickSetupDialog *dialog) : dialog(dialog) {
        dialog->setEnabled(false);
        QObject::connect(dialog, &QObject::destroyed, this, [this](QObject *) {
            this->dialog = nullptr;
        });
    }
    ~ProcessingGuard() {
        if (dialog) dialog->setEnabled(true);
    }
private:
    QuickSetupDialog *dialog;
};
}

void QuickSetupDialog::onResultClick(QAbstractButton *button)
{
    if (button == toAdvancedMode) {
        auto w = new MainWindow;
        w->show();
        close();
        return;
    }

    if (button == toPreferences){
        PreferencesDialog dialog;
        dialog.setWindowModality(Qt::ApplicationModal);
        dialog.exec();
        return;
    }

    csmm_quicksetup_detail::ProcessingGuard guard(this);

    bool shouldPatchRiivolutionVar = shouldPatchRiivolution();

    if (!Riivolution::validateRiivolutionName(ui->riivolutionPatchName->text())) {
        QMessageBox::critical(this, "Cannot save game", "Invalid Riivolution name: " + ui->riivolutionPatchName->text());
        return;
    }

    QString outputLoc;
    if (button == exportToWbfsIso) {
        outputLoc = QFileDialog::getSaveFileName(this, "Save WBFS/ISO", QString(), "Fortune Street disc files (*.wbfs *.iso *.ciso)", nullptr);
    } else {
        outputLoc = QFileDialog::getExistingDirectory(this, "Save Fortune Street Directory", nullptr);
        if (!outputLoc.isEmpty()) {
            if (!QDir(outputLoc).isEmpty()) {
                QMessageBox::critical(this, "Cannot select Fortune Street directory for saving", "Directory is not empty");
                return;
            }
        }
    }

    if (outputLoc.isEmpty()) {
        return;
    }

    // check if enough temporary disk space is available
    {
        QString path = "";
        QTemporaryDir saveDir(outputLoc);
        QStorageInfo saveLocation(saveDir.path());


        QSettings settings;
        QTemporaryDir tmp(settings.value("temporaryDirectory","").toString() + "/tmp");
        QTemporaryDir cacheDir(settings.value("networkCacheDirectory","").toString() + "/tmp");
        QStorageInfo cacheLocation(cacheDir.path());
        QStorageInfo temporaryLocation(tmp.path());

        int saveLocationAvailableMb = saveLocation.bytesAvailable()/1024/1024;
        int cacheLocationAvailableMb = cacheLocation.bytesAvailable()/1024/1024;
        int temporaryLocationAvailableMb = temporaryLocation.bytesAvailable()/1024/1024;

        QString lowStorageMessageCache = "There may not be enough space remaining on %1 to function as a network cache directory. It is recommended that your network cache drive has at least 10GB of free space before continuing. It currently has %2MB remaining.\n\nWould you like to proceed?";
        QString lowStorageMessageSave = "There may not be enough space remaining on %1 to store the new disc image.\n\nIt is recommended that the drive you're saving the Wii disc image to has at least 5GB of free space available. However, the drive currently has %2MB remaining.\n\nWould you like to proceed?";
        QString lowStorageMessageCacheAndSave = "%1 is configured as both a cache location as well as the export location for this disc image. As such, it is recommended that your drive has at least 15GB of available disk space before continuing. It currently has %2MB remaining.\n\nWould you like to proceed?";
        QString lowStorageMessageCacheAndTemporary = "%1 is configured as both a network cache location as well as the temporary directory location. As such, it is recommended that the drive has at least 15GB of available disk space before continuing. It currently has %2MB remaining.\n\nWould you like to proceed?";
        QString lowStorageMessageSaveAndTemporary = "%1 configured as both the save location for the disc image as well as the temporary directory to be used during extraction. As such, it is recommended that the drive has at least 15GB of available disk space before continuing. It currently has %2 remaining.\n\nWould you like to proceed?";
        QString lowStorageMessageTemporary = "There may not be enough space remaining on %1 to function as a temporary directory. This is the directory where downloaded archive files will be temporarily extracted before CSMM copies these files into the Wii disc image. As such, it is recommended that this drive has at least 10GB of free space before continuing. However, the drive currently has %2MB remaining.\n\nWould you like to proceed?";
        QString lowStorageMessageAll = "%1 is configured as a network cache directory, temporary directory, and the drive your disc image is being extracted to. As such, we recommend that this drive has at least 25GB of free space before continuing. However, it currently has %2MB remaining.\n\nWould you like to proceed?";

        if(saveLocation.device() == cacheLocation.device() && saveLocation.device() == temporaryLocation.device()){
            // if the cache, save, and temporary location drives are all the same, handle this accordingly
            if(saveLocationAvailableMb < 25000){
                if (QMessageBox::question(this, "Low Disk Space", lowStorageMessageAll.arg(saveLocation.displayName()).arg(saveLocationAvailableMb), QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
                    return;
            }
        }
        else if(saveLocation.device() == cacheLocation.device()){
            // the cache and the save location are on the same drive, handle accordingly
            if(saveLocationAvailableMb < 20000){
                if (QMessageBox::question(this, "Low Disk Space", lowStorageMessageCacheAndSave.arg(saveLocation.displayName()).arg(saveLocationAvailableMb), QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
                    return;
            }
        }
        else if(saveLocation.device() == temporaryLocation.device()){
            // the save and the temporary location are on the same drive, handle accordingly
            if(saveLocationAvailableMb < 15000){
                if (QMessageBox::question(this, "Low Disk Space", lowStorageMessageSaveAndTemporary.arg(saveLocation.displayName()).arg(saveLocationAvailableMb), QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
                    return;
            }
        }
        else if(cacheLocation.device() == temporaryLocation.device()){
            // the cache and the temporary location are on the same drive, handle accordingly
            if(cacheLocationAvailableMb < 15000){
                if (QMessageBox::question(this, "Low Disk Space", lowStorageMessageCacheAndTemporary.arg(cacheLocation.displayName()).arg(cacheLocationAvailableMb), QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
                    return;
            }
        }
        else {
            // find a way to make this known so we can handle _this_ accordingly
            if(cacheLocationAvailableMb < 10000){
                if (QMessageBox::question(this, "Low Disk Space", lowStorageMessageCache.arg(cacheLocation.displayName()).arg(cacheLocationAvailableMb), QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
                    return;
            }
            if(saveLocationAvailableMb < 5000){
                if (QMessageBox::question(this, "Low Disk Space", lowStorageMessageSave.arg(saveLocation.displayName()).arg(saveLocationAvailableMb), QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
                    return;
            }
            if(temporaryLocationAvailableMb < 10000){
                if (QMessageBox::question(this, "Low Disk Space", lowStorageMessageSave.arg(temporaryLocation.displayName()).arg(temporaryLocationAvailableMb), QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
                    return;
            }
        }
    }

    try {
        QSettings settings;
        QTemporaryDir importDir(settings.value("temporaryDirectory","").toString() + "/import");
        QTemporaryDir intermediateDir(settings.value("temporaryDirectory","").toString() + "/intermediate");

        QString targetGameDir = QFileInfo(outputLoc).isDir() ? outputLoc : intermediateDir.path();
        if (shouldPatchRiivolutionVar) {
            if (!QDir(targetGameDir).mkdir(ui->riivolutionPatchName->text())) {
                QMessageBox::critical(this, "Cannot save game", "Cannot create directory for putting patch files");
                return;
            }
            targetGameDir = QDir(targetGameDir).filePath(ui->riivolutionPatchName->text());
        }
        if (!importDir.isValid() || !intermediateDir.isValid()) {
            QMessageBox::critical(this, "Cannot save game", "Cannot create temporary directory.");
            return;
        }
        CSMMProgressDialog dialog("Starting process...", QString(), 0, 100, nullptr, Qt::WindowFlags(), true);
        dialog.setWindowModality(Qt::ApplicationModal);
        dialog.setWindowTitle("Creating Disc Image");
        // copy directory if folder, extract wbfs/iso if file
        if (QFileInfo(ui->inputGameLoc->text()).isDir()) {
            std::error_code error;
            std::filesystem::copy(ui->inputGameLoc->text().toStdU16String(), targetGameDir.toStdU16String(), std::filesystem::copy_options::recursive, error);
            if (error) {
                QMessageBox::critical(this, "Cannot save game", QString::fromStdString(error.message()));
                return;
            }
        } else {
            await(ExeWrapper::extractWbfsIso(ui->inputGameLoc->text(), targetGameDir));
        }
        if (shouldPatchRiivolutionVar) { // store vanilla game directory in intermediateDir for riivolution patching
            std::error_code error;
            std::filesystem::copy(targetGameDir.toStdU16String(), intermediateDir.path().toStdU16String(), std::filesystem::copy_options::recursive, error);
            if (error) {
                QMessageBox::critical(this, "Cannot save game", QString::fromStdString(error.message()));
                return;
            }
        }
        dialog.setValue(10);

        QVector<QString> modpackZips{ui->modpackZip->text()};
        for (int i=0; i<ui->additionalMods->count(); ++i) {
            modpackZips.append(ui->additionalMods->item(i)->text());
        }

        qInfo() << "Loading Modpacks: ";
        for (const QString &str : modpackZips) {
            qInfo() << str;
        }

        std::vector<std::shared_ptr<QTemporaryDir>> tmpDirs;

        for (int i = 0; i < modpackZips.size(); i++) {
            tmpDirs.emplace_back(std::make_shared<QTemporaryDir>(settings.value("temporaryDirectory", "").toString() + "/download"));
        }

        auto mods = ModLoader::importModpackCollection(modpackZips, tmpDirs);
        auto gameInstance = GameInstance::fromGameDirectory(targetGameDir, importDir.path());
        CSMMModpack modpack(gameInstance, mods.first.begin(), mods.first.end());
        modpack.load(targetGameDir);

        dialog.setValue(20);

        auto mapListFileIt = QDirIterator(mods.second[0]->path(), {"map[Ll]ist.yaml", "map[Ll]ist.yml"}, QDir::Files, QDirIterator::Subdirectories);
        if (!mapListFileIt.hasNext()) {
            QMessageBox::critical(this, "Cannot save game", "The maplist yaml was not found inside the modpack zip.");
            return;
        }

        mapListFileIt.next();
        Configuration::load(mapListFileIt.fileInfo().absoluteFilePath(), gameInstance.mapDescriptors(), QDir(importDir.path()), [&](double progress) {
            dialog.setValue(20 + (60 - 20) * progress);
        });

        dialog.setValue(60);

        modpack.save(targetGameDir, [&](double progress) {
            dialog.setValue(60 + (90 - 60) * progress);
        });

        dialog.setValue(90);

        // create wbfs/iso if file
        if (!QFileInfo(outputLoc).isDir()) {
            qInfo() << "Marker Code: " << ui->markerCode->text();
            qInfo() << "Is Separate Save Game: " << ui->separateSaveGame->isChecked();
            qInfo() << "Writing modified game image...";
            await(ExeWrapper::createWbfsIso(targetGameDir, outputLoc, ui->markerCode->text(), ui->separateSaveGame->isChecked()));
            if (std::find_if(mods.first.begin(), mods.first.end(), [](const auto &mod) { return mod->modId() == "wifiFix"; }) != mods.first.end()) {
                qInfo() << "Patching Wiimmfi...";
                dialog.setValue(95);
                await(ExeWrapper::patchWiimmfi(outputLoc));
            }
        }

        // riivolution stuff
        if (shouldPatchRiivolutionVar) {
            qInfo() << "Patching Riivolution...";
            dialog.setValue(95);
            Riivolution::write(intermediateDir.path(), outputLoc, gameInstance.addressMapper(), ui->riivolutionPatchName->text());
        }

        qInfo() << "Cleaning up...";

        // do some temporary directory cleanup
        for (auto &d: tmpDirs) {
            if(d->isValid()){
                d->remove();
            }
        }
        if(importDir.isValid()){
            importDir.remove();
        }
        if(intermediateDir.isValid()){
            intermediateDir.remove();
        }

        dialog.setValue(100);

        bool stayInCSMM = QMessageBox::question(this, "Success!", "The game was saved successfully. Would you like to exit?", QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes) == QMessageBox::No;
        if (stayInCSMM) {
            return;
        }
        close();
    } catch (const ProgressCanceled &) {
        // nothing to do
    } catch (const std::runtime_error &ex) {
        QMessageBox::critical(this, "Cannot save game", ex.what());
    }
}

void QuickSetupDialog::updateButtonBoxEnabled()
{
    bool enable = !ui->inputGameLoc->text().isEmpty() && !ui->modpackZip->text().isEmpty();
    exportToWbfsIso->setEnabled(enable && !ui->enableRiivolutionGroupBox->isChecked());
    exportToExtractedFolder->setEnabled(enable);
}

void QuickSetupDialog::accept()
{
    // no-op
}
