#include "quicksetupdialog.h"
#include "ui_quicksetupdialog.h"

#include <QtConcurrent>
#include <QFileDialog>
#include <QProgressDialog>
#include <QMessageBox>
#include <filesystem>

#include "lib/await.h"
#include "lib/exewrapper.h"
#include "lib/mods/modloader.h"
#include "lib/mods/csmmmodpack.h"
#include "lib/configuration.h"
#include "lib/riivolution.h"

QuickSetupDialog::QuickSetupDialog(const QString &defaultSaveId, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuickSetupDialog), defaultSaveId(defaultSaveId)
{
    ui->setupUi(this);

    ui->saveId->setText(defaultSaveId);

    connect(ui->chooseInputGameFolder, &QPushButton::clicked, this, [this](bool){
        auto dirname = QFileDialog::getExistingDirectory(this, "Open Fortune Street Directory");
        if (!dirname.isEmpty()) {
            ui->inputGameLoc->setText(dirname);
        }
    });
    connect(ui->chooseInputWbfsIso, &QPushButton::clicked, this, [this](bool){
        auto isoWbfs = QFileDialog::getOpenFileName(this, "Import WBFS/ISO", QString(), "Fortune Street disc files (*.wbfs *.iso *.ciso)");
        if (!isoWbfs.isEmpty()) {
            ui->inputGameLoc->setText(isoWbfs);
        }
    });
    connect(ui->chooseModpackFile, &QPushButton::clicked, this, [this](bool){
        auto file = QFileDialog::getOpenFileName(this, "Import mod pack", QString(), "modlist.txt or modpack zip files (*.txt;*.zip)");
        if (!file.isEmpty()) {
            ui->modpackFile->setText(file);
        }
    });
    connect(ui->chooseMapListFile, &QPushButton::clicked, this, [this](bool) {
        auto openFile = QFileDialog::getOpenFileName(this, "Load Map List", QString(), "CSMM Map List (*.yaml *.csv)");
        if (!openFile.isEmpty()) {
            ui->mapListFile->setText(openFile);
        }
    });
    connect(ui->chooseOutputFolder, &QPushButton::clicked, this, [this](bool){
        auto dirname = QFileDialog::getExistingDirectory(this, "Save Fortune Street Directory");
        if (!dirname.isEmpty()) {
            if (!QDir(dirname).isEmpty()) {
                QMessageBox::critical(this, "Cannot select Fortune Street directory for saving", "Directory is not empty");
                return;
            }
            ui->outputGameLoc->setText(dirname);
            ui->enableRiivolution->setEnabled(true);
            updateRiivolutionEnabled();
        }
    });
    connect(ui->chooseOutputWbfsIso, &QPushButton::clicked, this, [this](bool){
        auto saveFile = QFileDialog::getSaveFileName(this, "Save WBFS/ISO", QString(), "Fortune Street disc files (*.wbfs *.iso *.ciso)");
        if (!saveFile.isEmpty()) {
            ui->outputGameLoc->setText(saveFile);
            ui->enableRiivolution->setEnabled(false);
            updateRiivolutionEnabled();
        }
    });
    connect(ui->enableRiivolution, &QCheckBox::toggled, this, [this](bool) {
        updateRiivolutionEnabled();
    });
}

QuickSetupDialog::~QuickSetupDialog()
{
    delete ui;
}

void QuickSetupDialog::updateRiivolutionEnabled()
{
    bool riivolutionCheckboxEnabled = ui->enableRiivolution->isEnabled();
    bool riivolutionCheckboxChecked = ui->enableRiivolution->isChecked();
    bool riivolutionNameEnabled = riivolutionCheckboxEnabled && riivolutionCheckboxChecked;
    ui->riivolutionPatchName->setEnabled(riivolutionNameEnabled);
    ui->riivolutionPatchNameLabel->setEnabled(riivolutionNameEnabled);
}

bool QuickSetupDialog::shouldPatchRiivolution()
{
    return ui->enableRiivolution->isEnabled() && ui->enableRiivolution->isChecked();
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

void QuickSetupDialog::accept()
{
    bool shouldPatchRiivolutionVar = shouldPatchRiivolution();
    csmm_quicksetup_detail::ProcessingGuard guard(this);
    if (ui->inputGameLoc->text().isEmpty()) {
        QMessageBox::critical(this, "Cannot save game", "Input game ROM not specified");
        return;
    }
    if (ui->mapListFile->text().isEmpty()) {
        QMessageBox::critical(this, "Cannot save game", "Map list file not specified");
        return;
    }
    if (ui->outputGameLoc->text().isEmpty()) {
        QMessageBox::critical(this, "Cannot save game", "Output game ROM not specified");
        return;
    }
    if (!Riivolution::validateRiivolutionName(ui->riivolutionPatchName->text())) {
        QMessageBox::critical(this, "Cannot save game", "Invalid Riivolution name: " + ui->riivolutionPatchName->text());
        return;
    }

    try {
        QTemporaryDir importDir;
        QTemporaryDir intermediateDir;
        QString targetGameDir = QFileInfo(ui->outputGameLoc->text()).isDir()
                ? ui->outputGameLoc->text() : intermediateDir.path();
        if (shouldPatchRiivolutionVar) {
            if (!QDir(targetGameDir).mkdir(ui->riivolutionPatchName->text())) {
                QMessageBox::critical(this, "Cannot save game", "Cannot create directory for putting patch files");
                return;
            }
            targetGameDir = QDir(targetGameDir).filePath(ui->riivolutionPatchName->text());
        }
        if (!importDir.isValid() || !intermediateDir.isValid()) {
            QMessageBox::critical(this, "Cannot save game", "Cannot create temporary directory");
            return;
        }
        QProgressDialog dialog("Saving game to ROM", QString(), 0, 100);
        dialog.setWindowModality(Qt::ApplicationModal);
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

        auto mods = ModLoader::importModpackFile(ui->modpackFile->text());
        auto gameInstance = GameInstance::fromGameDirectory(targetGameDir, importDir.path());
        CSMMModpack modpack(gameInstance, mods.first.begin(), mods.first.end());
        modpack.load(targetGameDir);

        dialog.setValue(20);

        Configuration::load(ui->mapListFile->text(), gameInstance.mapDescriptors(), QDir(importDir.path()), [&](double progress) {
            dialog.setValue(20 + (60 - 20) * progress);
        });

        dialog.setValue(60);

        modpack.save(targetGameDir, [&](double progress) {
            dialog.setValue(60 + (90 - 60) * progress);
        });

        dialog.setValue(90);
        qInfo() << "writing ROM";

        // create wbfs/iso if file
        if (!QFileInfo(ui->outputGameLoc->text()).isDir()) {
            await(ExeWrapper::createWbfsIso(targetGameDir, ui->outputGameLoc->text(), ui->saveId->text()));
            if (std::find_if(mods.first.begin(), mods.first.end(), [](const auto &mod) { return mod->modId() == "wifiFix"; })) {
                qInfo() << "patching wiimmfi";
                dialog.setValue(95);
                await(ExeWrapper::patchWiimmfi(ui->outputGameLoc->text()));
            }
        }

        // riivolution stuff
        if (shouldPatchRiivolutionVar) {
            qInfo() << "patching riivolution";
            dialog.setValue(95);
            Riivolution::write(intermediateDir.path(), ui->outputGameLoc->text(), gameInstance.addressMapper(), ui->riivolutionPatchName->text());
        }

        dialog.setValue(100);

        QMessageBox::information(this, "Quick setup successful", "Save was successful.");

        QDialog::accept(); // fall back to parent accept which closes this dialog
    } catch (const std::runtime_error &ex) {
        QMessageBox::critical(this, "Cannot save game", ex.what());
    }
}
