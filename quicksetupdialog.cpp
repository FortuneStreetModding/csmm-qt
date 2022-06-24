#include "quicksetupdialog.h"
#include "ui_quicksetupdialog.h"

#include <QtConcurrent>
#include <QFileDialog>
#include <QProgressDialog>
#include <QMessageBox>
#include <filesystem>

#include "lib/asyncfuture.h"
#include "lib/await.h"
#include "lib/exewrapper.h"
#include "lib/mods/modloader.h"
#include "lib/mods/csmmmodpack.h"
#include "lib/configuration.h"

QuickSetupDialog::QuickSetupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuickSetupDialog)
{
    ui->setupUi(this);

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
        }
    });
    connect(ui->chooseOutputWbfsIso, &QPushButton::clicked, this, [this](bool){
        auto saveFile = QFileDialog::getSaveFileName(this, "Save WBFS/ISO", QString(), "Fortune Street disc files (*.wbfs *.iso *.ciso)");
        if (!saveFile.isEmpty()) {
            ui->outputGameLoc->setText(saveFile);
        }
    });

}

QuickSetupDialog::~QuickSetupDialog()
{
    delete ui;
}


void QuickSetupDialog::accept()
{
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

    try {
        QTemporaryDir intermediateDir;
        if (!intermediateDir.isValid()) {
            QMessageBox::critical(this, "Cannot save game", "Cannot create temporary directory");
            return;
        }
        QProgressDialog dialog;
        dialog.setMaximum(5);
        // copy directory if folder, extract wbfs/iso if file
        if (QFileInfo(ui->inputGameLoc->text()).isDir()) {
            std::error_code error;
            std::filesystem::copy(ui->inputGameLoc->text().toStdU16String(), intermediateDir.path().toStdU16String(), std::filesystem::copy_options::recursive, error);
            if (error) {
                QMessageBox::critical(this, "Cannot save game", QString::fromStdString(error.message()));
                return;
            }
        } else {
            await(ExeWrapper::extractWbfsIso(ui->inputGameLoc->text(), intermediateDir.path()));
        }

        dialog.setValue(1);

        auto mods = ModLoader::importModpackFile(ui->modpackFile->text());
        auto gameInstance = GameInstance::fromGameDirectory(intermediateDir.path());
        CSMMModpack modpack(gameInstance, mods.first.begin(), mods.first.end());
        modpack.load(intermediateDir.path());

        dialog.setValue(2);

        Configuration::load(ui->mapListFile->text(), gameInstance.mapDescriptors(), QDir(intermediateDir.path()));

        dialog.setValue(3);

        modpack.save(intermediateDir.path());

        dialog.setValue(4);

        // copy directory if folder, create wbfs/iso if file
        if (QFileInfo(ui->outputGameLoc->text()).isDir()) {
            std::error_code error;
            std::filesystem::copy(intermediateDir.path().toStdU16String(), ui->outputGameLoc->text().toStdU16String(), std::filesystem::copy_options::recursive, error);
            if (error) {
                QMessageBox::critical(this, "Cannot save game", QString::fromStdString(error.message()));
                return;
            }
        } else {
            await(ExeWrapper::createWbfsIso(intermediateDir.path(), ui->outputGameLoc->text(), "02")); // TODO allow saveid to be changed
        }

        dialog.setValue(5);

        QMessageBox::information(this, "Quick setup successful", "Save was successful.");

        QDialog::accept(); // fall back to parent accept which closes this dialog
    } catch (const std::runtime_error &ex) {
        QMessageBox::critical(this, "Cannot save game", ex.what());
    }
}
