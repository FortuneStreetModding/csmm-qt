#include "quicksetupdialog.h"
#include "lib/progresscanceled.h"
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

    ui->markerCode->setText(defaultMarkerCode);
    ui->separateSaveGame->setChecked(defaultSeparateSaveGame);

    connect(ui->chooseInputGameFolder, &QPushButton::clicked, this, [this](bool){
        auto dirname = QFileDialog::getExistingDirectory(this, "Open Fortune Street Directory");
        if (!dirname.isEmpty()) {
            ui->inputGameLoc->setText(dirname);
            updateButtonBoxEnabled();
        }
    });
    connect(ui->chooseInputWbfsIso, &QPushButton::clicked, this, [this](bool){
        auto isoWbfs = QFileDialog::getOpenFileName(this, "Import WBFS/ISO", QString(), "Fortune Street disc files (*.wbfs *.iso *.ciso)");
        if (!isoWbfs.isEmpty()) {
            ui->inputGameLoc->setText(isoWbfs);
            updateButtonBoxEnabled();
        }
    });
    connect(ui->chooseModpackZip, &QPushButton::clicked, this, [this](bool){
        auto file = QFileDialog::getOpenFileName(this, "Import mod pack", QString(), "Modpack zip file (*.zip)");
        if (!file.isEmpty()) {
            ui->modpackZip->setText(file);
            updateButtonBoxEnabled();
        }
    });
    connect(ui->addModZip, &QPushButton::clicked, this, [this](bool) {
        auto file = QFileDialog::getOpenFileName(this, "Import additional mod pack", QString(), "Modpack zip file (*.zip)");
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


    exportToWbfsIso = new QPushButton("Export to WBFS/ISO");
    exportToWbfsIso->setEnabled(false);
    exportToWbfsIso->setDefault(true);
    ui->buttonBox->addButton(exportToWbfsIso, QDialogButtonBox::AcceptRole);

    exportToExtractedFolder = new QPushButton("Export to Extracted Folder");
    exportToExtractedFolder->setEnabled(false);
    ui->buttonBox->addButton(exportToExtractedFolder, QDialogButtonBox::AcceptRole);

    toAdvancedMode = new QPushButton("Switch to Advanced CSMM");
    toAdvancedMode->setAutoDefault(false);
    ui->buttonBox->addButton(toAdvancedMode, QDialogButtonBox::ResetRole);

    connect(ui->enableRiivolution, &QCheckBox::toggled, this, [this](bool checked) {
        updateButtonBoxEnabled();
    });

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &QuickSetupDialog::onResultClick);

    updateButtonBoxEnabled();
}

QuickSetupDialog::~QuickSetupDialog()
{
    delete ui;
}

bool QuickSetupDialog::shouldPatchRiivolution()
{
    return ui->enableRiivolution->isChecked();
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

    csmm_quicksetup_detail::ProcessingGuard guard(this);

    bool shouldPatchRiivolutionVar = shouldPatchRiivolution();

    if (!Riivolution::validateRiivolutionName(ui->riivolutionPatchName->text())) {
        QMessageBox::critical(this, "Cannot save game", "Invalid Riivolution name: " + ui->riivolutionPatchName->text());
        return;
    }

    QString outputLoc;
    if (button == exportToWbfsIso) {
        outputLoc = QFileDialog::getSaveFileName(this, "Save WBFS/ISO", QString(), "Fortune Street disc files (*.wbfs *.iso *.ciso)");
    } else {
        outputLoc = QFileDialog::getExistingDirectory(this, "Save Fortune Street Directory");
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
        QTemporaryDir tmp;
        QStorageInfo storageInfo(tmp.path());
        int availableMb = storageInfo.bytesAvailable()/1024/1024;
        if (availableMb < 5000) {
            if (QMessageBox::question(this, "Save",
                                  QString("There is less than 5 GB of space left on %1\nCSMM stores temporary files and needs enough disk space to function properly.").arg(storageInfo.displayName()),
                                  QMessageBox::Ok|QMessageBox::Cancel) == QMessageBox::Cancel)
                return;
        }
    }

    try {
        QTemporaryDir importDir;
        QTemporaryDir intermediateDir;

        QString targetGameDir = QFileInfo(outputLoc).isDir()
                ? outputLoc : intermediateDir.path();
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
        CSMMProgressDialog dialog("Saving game to ROM", QString(), 0, 100, nullptr, Qt::WindowFlags(), true);
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

        QVector<QString> modpackZips{ui->modpackZip->text()};
        for (int i=0; i<ui->additionalMods->count(); ++i) {
            modpackZips.append(ui->additionalMods->item(i)->text());
        }
        auto mods = ModLoader::importModpackCollection(modpackZips);
        auto gameInstance = GameInstance::fromGameDirectory(targetGameDir, importDir.path());
        CSMMModpack modpack(gameInstance, mods.first.begin(), mods.first.end());
        modpack.load(targetGameDir);

        dialog.setValue(20);

        auto mapListFileIt = QDirIterator(mods.second[0].path(), {"map[Ll]ist.yaml", "map[Ll]ist.yml"}, QDir::Files, QDirIterator::Subdirectories);
        if (!mapListFileIt.hasNext()) {
            QMessageBox::critical(this, "Cannot save game", "Map list yaml not found in modpack zip");
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
        qInfo() << "writing ROM";

        // create wbfs/iso if file
        if (!QFileInfo(outputLoc).isDir()) {
            await(ExeWrapper::createWbfsIso(targetGameDir, outputLoc, ui->markerCode->text(), ui->separateSaveGame->isChecked()));
            if (std::find_if(mods.first.begin(), mods.first.end(), [](const auto &mod) { return mod->modId() == "wifiFix"; })) {
                qInfo() << "patching wiimmfi";
                dialog.setValue(95);
                await(ExeWrapper::patchWiimmfi(outputLoc));
            }
        }

        // riivolution stuff
        if (shouldPatchRiivolutionVar) {
            qInfo() << "patching riivolution";
            dialog.setValue(95);
            Riivolution::write(intermediateDir.path(), outputLoc, gameInstance.addressMapper(), ui->riivolutionPatchName->text());
        }

        dialog.setValue(100);

        QMessageBox::information(this, "Quick setup successful", "Save was successful.");

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
    exportToWbfsIso->setEnabled(enable && !ui->enableRiivolution->isChecked());
    exportToExtractedFolder->setEnabled(enable);
}

void QuickSetupDialog::accept()
{
    // no-op
}
