#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "lib/filesystem.hpp"
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QtConcurrent>
#include <QInputDialog>
#include "lib/asyncfuture.h"
#include "lib/exewrapper.h"
#include "lib/importexportutils.h"
#include "downloadclidialog.h"
#include "validationerrordialog.h"
#include "lib/configuration.h"
#include "lib/downloadtools.h"
#include "lib/datafileset.h"
#include "lib/mods/defaultmodlist.h"
#include "lib/mods/modloader.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), manager(new QNetworkAccessManager(this)), modList(DefaultModList::defaultModList())
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    ui->tableWidget->setGameDirectoryFunction([&]() { return windowFilePath(); });
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openDir);
    connect(ui->actionImport_WBFS_ISO, &QAction::triggered, this, &MainWindow::openIsoWbfs);
    connect(ui->actionExport_to_Folder, &QAction::triggered, this, &MainWindow::exportToFolder);
    connect(ui->actionExport_to_WBFS_ISO, &QAction::triggered, this, &MainWindow::exportIsoWbfs);
    connect(ui->action_Re_Download_External_Tools, &QAction::triggered, this, [&]() {
        checkForRequiredFiles(true);
    });
    connect(ui->actionCSMM_Help, &QAction::triggered, this, [&]() {
        QDesktopServices::openUrl(QUrl("https://github.com/FortuneStreetModding/fortunestreetmodding.github.io/wiki/CSMM-User-Manual"));
    });
    connect(ui->actionValidate, &QAction::triggered, this, &MainWindow::validateMaps);
    connect(ui->actionSave_map_list_csv, &QAction::triggered, this, &MainWindow::saveMapList);
    connect(ui->actionLoad_map_list_csv, &QAction::triggered, this, &MainWindow::loadMapList);
    connect(ui->actionItast_csmm_brsar, &QAction::triggered, this, &MainWindow::saveCleanItastCsmmBrsar);
    connect(ui->addMap, &QPushButton::clicked, this, [&](bool) { ui->tableWidget->appendMapDescriptor(MapDescriptor()); });
    connect(ui->removeMap, &QPushButton::clicked, this, [&](bool) {
        if (QMessageBox::question(this, "Remove Map(s)", "Are you sure you want to remove the selected maps?") == QMessageBox::Yes) {
            ui->tableWidget->removeSelectedMapDescriptors();
        }
    });
    connect(ui->actionPatch_SaveId, &QAction::triggered, this, [&]() {
        bool ok;
        QString text = QInputDialog::getText(this, tr("Enter SaveId."),
                                             tr("- 2 characters.\n- Digits and uppercase letters only.\n- 01 is vanilla game. The save file is then shared with vanilla game.\n- 02 is csmm default."), QLineEdit::Normal,
                                             getSaveId(), &ok);
        if (ok && !text.isEmpty()) {
            if(text.length() != 2) {
                QMessageBox::critical(this, "Save ID", "The input must be two characters");
            } else {
                ui->actionPatch_SaveId->setText(QString("Patch SaveId (SaveId=%1)").arg(text.toUpper()));
                ui->actionPatch_SaveId->setData(text.toUpper());
            }
        }
        ui->actionPatch_SaveId->setChecked(getSaveId() != "01");
    });
    connect(ui->actionExport_default_modlists_txt, &QAction::triggered, this, [&]() {
        auto saveFile = QFileDialog::getSaveFileName(this, "Export default modlist.txt", "modlist.txt", "Mod List (*.txt)");

        QSaveFile fileObj(saveFile);

        if (!fileObj.open(QFile::WriteOnly)) {
            QMessageBox::critical(this, "Error exporting modlist.txt", "Error opening file for saving");
        }

        auto defaultModList = DefaultModList::defaultModList();

        QTextStream stream(&fileObj);
        stream.setCodec("UTF-8");

        for (auto &mod: defaultModList) {
            stream << mod->modId() << "\n";
        }

        fileObj.commit();
    });
    connect(ui->actionImport_mod_pack, &QAction::triggered, this, [&]() {
        auto dir = QFileDialog::getExistingDirectory(this, "Import mod pack");

        try {
            modList = ModLoader::importModpack(dir);
        } catch (const std::runtime_error &error) {
            QMessageBox::critical(this, "Error importing modpack", QString("Error importing modpack:\n%1").arg(error.what()));
            PyErr_Clear();
        } catch (...) { // TODO add catches for specific errors
            QMessageBox::critical(this, "Error importing modpack", "Error importing modpack");
        }
    });
}

QString MainWindow::getSaveId() {
    if(ui->actionPatch_SaveId->data().isNull())
        return "02";
    return ui->actionPatch_SaveId->data().toString().toUpper();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::saveMapList() {
    auto saveFile = QFileDialog::getSaveFileName(this, "Save Map List", "mapList.csv", "CSMM Map List (*.csv)");
    if (saveFile.isEmpty()) return;
    QFileInfo saveFileInfo(saveFile);
    if(saveFileInfo.exists()) {
        QFile::remove(saveFile);
    }
    std::vector<MapDescriptor> descriptors;
    auto descriptorPtrs = ui->tableWidget->getDescriptors();
    std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(descriptors), [](auto &ptr) { return *ptr; });
    Configuration::save(saveFile, descriptors);
}

void MainWindow::loadMapList() {
    auto openFile = QFileDialog::getOpenFileName(this, "Load Map List", QString(), "CSMM Map List (*.csv)");
    if (openFile.isEmpty()) return;
    QFileInfo openFileInfo(openFile);
    if(!openFileInfo.exists()) {
        QMessageBox::critical(this, "Open Map List", QString("Error loading the map list: %1").arg(openFile));
    }
    std::vector<MapDescriptor> descriptors;
    auto descriptorPtrs = ui->tableWidget->getDescriptors();
    std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(descriptors), [](auto &ptr) { return *ptr; });
    ui->statusbar->showMessage("Warning: This operation will import the maps in the map list one by one. Depending on the size of the map list, this can take a while and CSMM may freeze.");
    ui->statusbar->repaint();
    try {
        Configuration::load(openFile, descriptors, tempGameDir->path());
        loadDescriptors(descriptors);
        ui->tableWidget->dirty = true;
        ui->statusbar->showMessage("Map list load completed");
        ui->statusbar->repaint();
    } catch (const ImportExportUtils::Exception &exception) {
        QMessageBox::critical(this, "Import .yaml", QString("Error loading the map: %1").arg(exception.getMessage()));
    } catch (const YAML::Exception &exception) {
        QMessageBox::critical(this, "Import .yaml", QString("Error loading the map: %1").arg(exception.what()));
    }
}

void MainWindow::saveCleanItastCsmmBrsar() {
    auto openFile = QFileDialog::getOpenFileName(this, "Open vanilla Itast.brsar", "Itast.brsar", "Vanilla Fortune Street Binary Sound Archive (*.brsar)");
    if (openFile.isEmpty()) return;
    //QFileInfo openFileInfo(openFile);

    if (ImportExportUtils::fileSha1(openFile) != ImportExportUtils::getSha1OfVanillaFileName(SOUND_FOLDER + "/Itast.brsar")) {
        QMessageBox::information(this, "Wrong Itast.brsar", QString("The provided file %1 is not a vanilla Itast.brsar").arg(openFile));
        return;
    }

    auto saveFile = QFileDialog::getSaveFileName(this, "Save clean Itast.csmm.brsar", "Itast.csmm.brsar", "CSMM Fortune Street Binary Sound Archive (*.brsar)");
    if (saveFile.isEmpty()) return;

    QString errors = ImportExportUtils::applyBspatch(openFile, saveFile, ":/" + SOUND_FOLDER + "/Itast.brsar.bsdiff");
    if(!errors.isEmpty()) {
        QMessageBox::critical(this, "Open", QString("Errors occurred when applying Itast.brsar.bsdiff patch to file %1:\n%2").arg(openFile, errors));
    }

    QMessageBox::information(this, "Save", QString("Saved to %1").arg(saveFile));
}

void MainWindow::openDir() {
    auto newTempGameDir = QSharedPointer<QTemporaryDir>::create();
    if (!newTempGameDir->isValid()) {
        QMessageBox::critical(this, "Open Game Directory", "The temporary directory used for copying the game directory could not be created");
        return;
    }
    QString dirname = QFileDialog::getExistingDirectory(this, "Open Fortune Street Directory");
    if (dirname.isEmpty()) {
        return;
    }

    auto progress = QSharedPointer<QSharedPointer<QProgressDialog>>::create(nullptr);

    auto copyTask = QtConcurrent::run([=]() {
        std::error_code error;
        ghc::filesystem::copy(dirname.toStdU16String(), newTempGameDir->path().toStdU16String(), ghc::filesystem::copy_options::recursive, error);
        return error;
    });

    AsyncFuture::observe(checkForRequiredFiles())
            .subscribe([=]() {
        if (!ImportExportUtils::isMainDolVanilla(QDir(dirname))) {
            auto btn = QMessageBox::warning(this, "Non-vanilla main.dol detected",
                                 "CSMM has detected a non-vanilla main.dol; modifying a main.dol that has already been patched with CSMM is not fully supported. Continue anyway?",
                                 QMessageBox::Yes | QMessageBox::No);
            if (btn != QMessageBox::Yes) {
                return;
            }
        }

        try {
            *progress = QSharedPointer<QProgressDialog>::create("Importing folder", QString(), 0, 2, this);
            (*progress)->setWindowModality(Qt::WindowModal);
            (*progress)->setValue(0);

            auto gameInstance = GameInstance::fromGameDirectory(dirname);
            CSMMModpack modpack(gameInstance, modList.begin(), modList.end());
            modpack.load(dirname);
            (*progress)->setValue(1);
            auto errorCode = await(copyTask);
            (*progress)->setValue(2);
            if (errorCode) {
                QMessageBox::critical(this, "Error loading game", QString("Error copying files to temporary working directory"));
                return;
            }
            loadDescriptors(gameInstance.mapDescriptors());
            setWindowFilePath(newTempGameDir->path());
            tempGameDir = newTempGameDir;
        } catch (const std::runtime_error &e) {
            *progress = nullptr;
            QMessageBox::critical(this, "Error loading game", QString("Error loading game: %1").arg(e.what()));
            PyErr_Clear();
        }
    });
}

void MainWindow::openIsoWbfs() {
    auto newTempGameDir = QSharedPointer<QTemporaryDir>::create();
    if (!newTempGameDir->isValid()) {
        QMessageBox::critical(this, "Import WBFS/ISO", "The temporary directory used for importing disc images could not be created");
        return;
    }
    QString isoWbfs = QFileDialog::getOpenFileName(this, "Import WBFS/ISO", QString(), "Fortune Street disc files (*.wbfs *.iso *.ciso)");
    if (isoWbfs.isEmpty()) return;

    auto progress = QSharedPointer<QSharedPointer<QProgressDialog>>::create(nullptr);

    AsyncFuture::observe(checkForRequiredFiles())
            .subscribe([=]() {
        *progress = QSharedPointer<QProgressDialog>::create("Importing WBFS/ISO…", QString(), 0, 2, this);
        (*progress)->setWindowModality(Qt::WindowModal);
        (*progress)->setValue(0);
        return ExeWrapper::extractWbfsIso(isoWbfs, newTempGameDir->path());
    }).subscribe([=]() {
        (*progress)->setValue(1);
        if (!ImportExportUtils::isMainDolVanilla(newTempGameDir->path())) {
            auto btn = QMessageBox::warning(this, "Non-vanilla main.dol detected",
                                 "CSMM has detected a non-vanilla main.dol; modifying a main.dol that has already been patched with CSMM is not fully supported. Continue anyway?",
                                 QMessageBox::Yes | QMessageBox::No);
            if (btn != QMessageBox::Yes) {
                return;
            }
        }
        auto dirname = newTempGameDir->path();

        try {
            auto gameInstance = GameInstance::fromGameDirectory(dirname);
            CSMMModpack modpack(gameInstance, modList.begin(), modList.end());
            modpack.load(dirname);
            (*progress)->setValue(2);
            loadDescriptors(gameInstance.mapDescriptors());
            setWindowFilePath(newTempGameDir->path());
            tempGameDir = newTempGameDir;
        } catch (const std::runtime_error &e) {
            *progress = nullptr;
            QMessageBox::critical(this, "Error loading game", QString("Error loading game: %1").arg(e.what()));
            PyErr_Clear();
        }
    });
}

void MainWindow::loadDescriptors(const std::vector<MapDescriptor> &descriptors) {
    ui->tableWidget->clearDescriptors();
    for (auto &descriptor: descriptors) {
        ui->tableWidget->appendMapDescriptor(descriptor);
    }
    ui->tableWidget->dirty = false;
    ui->mapToolbar->setEnabled(true);
    ui->actionValidate->setEnabled(true);
    ui->actionLoad_map_list_csv->setEnabled(true);
    ui->actionSave_map_list_csv->setEnabled(true);
    ui->actionExport_to_Folder->setEnabled(true);
    ui->actionExport_to_WBFS_ISO->setEnabled(true);
}

QFuture<void> MainWindow::checkForRequiredFiles(bool alwaysAsk) {
    if (alwaysAsk || !DownloadTools::requiredFilesAvailable()) {
        DownloadCLIDialog dialog(WIT_URL, WSZST_URL, this);
        if (dialog.exec() == QDialog::Accepted) {
            auto fut = DownloadTools::downloadAllRequiredFiles(manager, [&](const QString &error) {
                QMessageBox::critical(this, "Download", error);
            }, dialog.getWitURL(), dialog.getWszstURL());
            return AsyncFuture::observe(fut).subscribe([=]() {
                QMessageBox::information(this, "Download", "Successfuly downloaded and extracted the programs.");
            }).future();
        }
        auto def = AsyncFuture::deferred<void>();
        def.cancel();
        return def.future();
    }
    auto def = AsyncFuture::deferred<void>();
    def.complete();
    return def.future();
}

void MainWindow::exportToFolder() {
    auto saveDir = QFileDialog::getExistingDirectory(this, "Save to Fortune Street Directory");
    if (saveDir.isEmpty()) return;
    if (!QDir(saveDir).isEmpty()) {
        QMessageBox::critical(this, "Save", "Directory is non-empty");
        return;
    }

    if (!ui->tableWidget->dirty) {
        if (QMessageBox::question(this, "Clean Export", "It seems you haven't made any changes.\nDo you want to make a clean export without letting CSMM make any game code changes and without applying any of the optional patches? ") == QMessageBox::Yes) {
            auto progress = QSharedPointer<QProgressDialog>::create("Saving…", QString(), 0, 2, this);
            progress->setWindowModality(Qt::WindowModal);
            progress->setValue(0);

            // need to use utf 16 b/c windows behaves strangely w/ utf 8
            auto copyTask = QtConcurrent::run([=] {
                std::error_code error;
                ghc::filesystem::copy(windowFilePath().toStdU16String(), saveDir.toStdU16String(), ghc::filesystem::copy_options::recursive, error);
                return !error;
            });
            auto fut = AsyncFuture::observe(copyTask).subscribe([=](bool result) {
                if (!result) {
                    QMessageBox::critical(this, "Save", "Could not copy game data.");
                } else {
                    progress->setValue(2);
                    QMessageBox::information(this, "Save", "Saved successfuly.");
                }
            });
            return;
        }
    }

    if (ui->actionPatch_Wiimmfi->isChecked()) {
        ui->statusbar->showMessage("Warning: Wiimmfi patching is not supported when exporting to a folder.");
    }

    auto progress = QSharedPointer<QProgressDialog>::create("Saving…", QString(), 0, 2, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setValue(0);

    auto copyTask = QtConcurrent::run([=] {
        std::error_code error;
        ghc::filesystem::copy(windowFilePath().toStdU16String(), saveDir.toStdU16String(), ghc::filesystem::copy_options::recursive, error);
        return !error;
    });
    auto descriptors = QSharedPointer<std::vector<MapDescriptor>>::create();
    auto fut = AsyncFuture::observe(copyTask)
            .subscribe([=](bool result) {
        if (!result) {
            QMessageBox::critical(this, "Save", "Could not copy game data to temporary directory for modifying");
            return;
        }
        progress->setValue(1);
        auto descriptorPtrs = ui->tableWidget->getDescriptors();
        std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(*descriptors), [](auto &ptr) { return *ptr; });
        try {
            auto gameInstance = GameInstance::fromGameDirectory(saveDir, *descriptors);
            CSMMModpack modpack(gameInstance, modList.begin(), modList.end());
            modpack.save(saveDir);
            progress->setValue(2);
            QMessageBox::information(this, "Save", "Saved successfuly.");

            // reload map descriptors
            int idx = 0;
            for (auto &descriptor: gameInstance.mapDescriptors()) {
                ui->tableWidget->loadRowWithMapDescriptor(idx++, descriptor);
            }
        } catch (const std::runtime_error &exception) {
            QMessageBox::critical(this, "Export", QString("Export failed: %1").arg(exception.what()));
            PyErr_Clear();
            throw exception;
        }
    });
}

void MainWindow::exportIsoWbfs() {
    auto saveFile = QFileDialog::getSaveFileName(this, "Export WBFS/ISO", QString(), "Fortune Street disc files (*.wbfs *.iso *.ciso)");
    if (saveFile.isEmpty()) return;

    auto intermediateResults = QSharedPointer<QTemporaryDir>::create();
    if (!intermediateResults->isValid()) {
        QMessageBox::critical(this, "Export WBFS/ISO", "Could not create a temporary directory for patching");
        return;
    }

    if (!ui->tableWidget->dirty) {
        if (QMessageBox::question(this, "Clean Export", "It seems you haven't made any changes.\nDo you want to make a clean export without letting CSMM make any game code changes and without applying any of the optional patches?") == QMessageBox::Yes) {
            auto progress = QSharedPointer<QProgressDialog>::create("Saving…", QString(), 0, 2, this);
            progress->setWindowModality(Qt::WindowModal);
            progress->setValue(0);
            auto fut = AsyncFuture::observe(ExeWrapper::createWbfsIso(windowFilePath(), saveFile, "01")).subscribe([=]() {
                QMessageBox::information(this, "Export", "Exported successfuly.");
                progress->setValue(2);
            });
            return;
        }
    }

    auto descriptors = QSharedPointer<std::vector<MapDescriptor>>::create();

    auto progress = QSharedPointer<QProgressDialog>::create("Exporting to image…", QString(), 0, 4, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setValue(0);

    QString intermediatePath = intermediateResults->path();
    auto copyTask = QtConcurrent::run([=] {
        std::error_code error;
        ghc::filesystem::copy(windowFilePath().toStdU16String(), intermediatePath.toStdU16String(), ghc::filesystem::copy_options::recursive, error);
        return !error;
    });
    auto fut = AsyncFuture::observe(copyTask)
            .subscribe([=](bool result) {
        if (!result) {
            auto def = AsyncFuture::deferred<void>();
            def.complete();
            return def.future();
        }
        progress->setValue(1);
        auto descriptorPtrs = ui->tableWidget->getDescriptors();
        std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(*descriptors), [](auto &ptr) { return *ptr; });
        try {
            auto gameInstance = GameInstance::fromGameDirectory(intermediatePath, *descriptors);
            CSMMModpack modpack(gameInstance, modList.begin(), modList.end());
            modpack.save(intermediatePath);
            *descriptors = gameInstance.mapDescriptors();
            progress->setValue(2);
            return ExeWrapper::createWbfsIso(intermediatePath, saveFile, getSaveId());
        } catch (const std::runtime_error &exception) {
            QMessageBox::critical(this, "Export", QString("Export failed: %1").arg(exception.what()));
            PyErr_Clear();
            throw exception;
        }
    }).subscribe([=]() {
        progress->setValue(3);
        if (std::find_if(modList.begin(), modList.end(), [](const auto &mod) { return mod->modId() == "wifiFix"; })) {
            return ExeWrapper::patchWiimmfi(saveFile);
        }
        auto def = AsyncFuture::deferred<void>();
        def.complete();
        return def.future();
    }).subscribe([=]() {
        (void)intermediateResults; // keep temporary directory active while creating wbfs/iso

        progress->setValue(4);
        QMessageBox::information(this, "Export", "Exported successfuly.");

        // reload map descriptors
        int idx = 0;
        for (auto &descriptor: *descriptors) {
            ui->tableWidget->loadRowWithMapDescriptor(idx++, descriptor);
        }
    });
}

void MainWindow::validateMaps() {
    QStringList errorMsgs;
    std::vector<MapDescriptor> descriptors;
    auto descriptorPtrs = ui->tableWidget->getDescriptors();
    std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(descriptors), [](auto &ptr) { return *ptr; });

    short easyPracticeBoard, standardPracticeBoard;
    getPracticeBoards(descriptors, easyPracticeBoard, standardPracticeBoard, errorMsgs);

    auto descriptorToMapSet = getMapSets(descriptors, errorMsgs);
    auto mapSets = descriptorToMapSet.values();
    std::sort(mapSets.begin(), mapSets.end());
    mapSets.erase(std::unique(mapSets.begin(), mapSets.end()), mapSets.end());

    for (int mapSet: qAsConst(mapSets)) {
        auto descriptorToZone = getMapZones(descriptors, mapSet, errorMsgs);
        auto zones = descriptorToZone.values();
        std::sort(zones.begin(), zones.end());
        zones.erase(std::unique(zones.begin(), zones.end()), zones.end());

        for (int zone: qAsConst(zones)) {
            getMapOrderings(descriptors, mapSet, zone, errorMsgs);
        }
    }

    if (errorMsgs.isEmpty()) {
        QMessageBox::information(this, "Validation", "Validation passed.");
    } else {
        (new ValidationErrorDialog(errorMsgs, this))->exec();
    }
}
