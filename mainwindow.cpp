#include "mainwindow.h"
#include "preferencesdialog.h"
#include "ui_mainwindow.h"

#include <filesystem>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QtConcurrent>
#include <QInputDialog>
#include "lib/configuration.h"
#include "lib/csmmnetworkmanager.h"
#include "lib/asyncfuture.h"
#include "lib/exewrapper.h"
#include "lib/importexportutils.h"
#include "validationerrordialog.h"
#include "lib/datafileset.h"
#include "lib/mods/defaultmodlist.h"
#include "lib/mods/modloader.h"
#include "lib/riivolution.h"
#include "quicksetupdialog.h"
#include "csmmprogressdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), modList(DefaultModList::defaultModList())
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    ui->tableWidget->setGameDirectoryFunction([&]() { return windowFilePath(); });
    ui->tableWidget->setImportDirectoryFunction([&]() { return importDir->path(); });
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openDir);
    connect(ui->actionImport_WBFS_ISO, &QAction::triggered, this, &MainWindow::openIsoWbfs);
    connect(ui->actionExport_to_Folder, &QAction::triggered, this, [&]() {
        exportToFolder(false);
    });
    connect(ui->actionExport_to_WBFS_ISO, &QAction::triggered, this, &MainWindow::exportIsoWbfs);
    connect(ui->actionExport_to_Riivolution, &QAction::triggered, this, [&]() {
        exportToFolder(true);
    });
    connect(ui->actionShow_CSMM_Network_Cache_In_File_System, &QAction::triggered, this, [&]() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(CSMMNetworkManager::networkCacheDir()));
    });
    connect(ui->actionOpen_Log_Folder, &QAction::triggered, this, [&]() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)));
    });
    connect(ui->actionCSMM_Help, &QAction::triggered, this, [&]() {
        QDesktopServices::openUrl(QUrl("https://github.com/FortuneStreetModding/fortunestreetmodding.github.io/wiki/CSMM-User-Manual"));
    });
    connect(ui->actionValidate, &QAction::triggered, this, &MainWindow::validateMaps);
    connect(ui->actionSave_map_list_csv, &QAction::triggered, this, &MainWindow::saveMapList);
    connect(ui->actionLoad_map_list_csv, &QAction::triggered, this, &MainWindow::loadMapList);
    connect(ui->actionItast_csmm_brsar, &QAction::triggered, this, &MainWindow::saveCleanItastCsmmBrsar);
    connect(ui->addMap, &QPushButton::clicked, this, [&](bool) { ui->tableWidget->appendMapDescriptor(MapDescriptor()); });
    connect(ui->duplicateMap, &QPushButton::clicked, this, [&](bool) {
        ui->tableWidget->duplicateSelectedMapDescriptors();
    });
    connect(ui->removeMap, &QPushButton::clicked, this, [&](bool) {
        if (QMessageBox::question(this, "Remove Map(s)", "Are you sure you want to remove the selected maps?") == QMessageBox::Yes) {
            ui->tableWidget->removeSelectedMapDescriptors();
        }
    });
    connect(ui->actionPatch_MarkerCode, &QAction::triggered, this, [&]() {
        bool ok;
        QString text = QInputDialog::getText(this, tr("Enter MarkerCode."),
                                             tr("- 2 characters.\n- Digits and uppercase letters only.\n- 01 is vanilla game.\n- 02 is csmm default."), QLineEdit::Normal,
                                             getMarkerCode(), &ok);
        if (ok && !text.isEmpty()) {
            if(text.length() != 2) {
                QMessageBox::critical(this, "MarkerCode", "The input must be two characters");
            } else {
                ui->actionPatch_MarkerCode->setText(QString("Patch MarkerCode (MarkerCode=%1)").arg(text.toUpper()));
                ui->actionPatch_MarkerCode->setData(text.toUpper());
            }
        }
        ui->actionPatch_MarkerCode->setChecked(getMarkerCode() != "01");
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
    connect(ui->importModPack, &QPushButton::clicked, this, [&](bool) {
        auto file = QFileDialog::getOpenFileName(this, "Import mod pack", QString(), "modlist.txt or modpack zip files (*.txt;*.zip)");

        if (file.isEmpty()) {
            return;
        }

        try {
            std::tie(modList, tempModpackDir) = ModLoader::importModpackFile(file);
            updateModListWidget();
            QMessageBox::information(this, "Import mod pack", "Modpack successfully imported.");
        } catch (const std::runtime_error &error) {
            QMessageBox::critical(this, "Error importing modpack", QString("Error importing modpack:\n%1").arg(error.what()));
            PyErr_Clear();
        }
    });
    connect(ui->quickSetup, &QPushButton::clicked, this, [&](bool) {
        auto dialog = new QuickSetupDialog(getMarkerCode(), getSeparateSaveGame());
        dialog->show();
        close();
    });
    connect(ui->actionPreferences, &QAction::triggered, this, [&]() {
        PreferencesDialog dialog;
        dialog.exec();
    });
    updateModListWidget();
}

QString MainWindow::getMarkerCode() {
    if(ui->actionPatch_MarkerCode->data().isNull())
        return "02";
    return ui->actionPatch_MarkerCode->data().toString().toUpper();
}

bool MainWindow::getSeparateSaveGame() {
    return ui->actionSeparate_Save_Game->isChecked();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::saveMapList() {
    auto saveFile = QFileDialog::getSaveFileName(this, "Save Map List", "mapList.yaml", "CSMM Map List (*.yaml)");
    if (saveFile.isEmpty()) return;
    std::vector<MapDescriptor> descriptors;
    auto descriptorPtrs = ui->tableWidget->getDescriptors();
    std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(descriptors), [](auto &ptr) { return *ptr; });
    try {
        Configuration::save(saveFile, descriptors);
    } catch (const std::runtime_error &e) {
         QMessageBox::critical(this, "Save Map List", QString("Error saving the map list: %1").arg(e.what()));
    }
}

void MainWindow::loadMapList() {
    auto openFile = QFileDialog::getOpenFileName(this, "Load Map List", QString(), "CSMM Map List (*.yaml *.csv)");
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
    CSMMProgressDialog progressDialog("Importing map list", QString(), 0, 100);
    progressDialog.setWindowModality(Qt::ApplicationModal);
    try {
        Configuration::load(openFile, descriptors, importDir->path(), [&](double progress) {
            progressDialog.setValue(100 * progress);
        });
        loadDescriptors(descriptors);
        ui->tableWidget->dirty = true;
        ui->statusbar->showMessage("Map list load completed");
        ui->statusbar->repaint();
    } catch (const std::runtime_error &exception) {
        QMessageBox::critical(this, "Import Map List", QString("Error importing the map list: %1").arg(exception.what()));
    }
}

void MainWindow::updateModListWidget() {
    ui->modListWidget->clear();
    QStringList modStrs;
    for (auto &mod: modList) {
        modStrs.append(mod->modId());
    }
    ui->modListWidget->addItems(modStrs);
}

void MainWindow::saveCleanItastCsmmBrsar() {
    auto openFile = QFileDialog::getOpenFileName(this, "Open vanilla Itast.brsar", "Itast.brsar", "Vanilla Fortune Street Binary Sound Archive (*.brsar)");
    if (openFile.isEmpty()) return;
    //QFileInfo openFileInfo(openFile);

    if (ImportExportUtils::fileSha1(openFile) != ImportExportUtils::getSha1OfVanillaFileName(ITAST_BRSAR)) {
        QMessageBox::information(this, "Wrong Itast.brsar", QString("The provided file %1 is not a vanilla Itast.brsar").arg(openFile));
        return;
    }

    auto saveFile = QFileDialog::getSaveFileName(this, "Save clean Itast.csmm.brsar", "Itast.csmm.brsar", "CSMM Fortune Street Binary Sound Archive (*.brsar)");
    if (saveFile.isEmpty()) return;

    QString errors = ImportExportUtils::applyBspatch(openFile, saveFile, ":/" + ITAST_BRSAR + ".bsdiff");
    if(!errors.isEmpty()) {
        QMessageBox::critical(this, "Open", QString("Errors occurred when applying Itast.brsar.bsdiff patch to file %1:\n%2").arg(openFile, errors));
    }

    QMessageBox::information(this, "Save", QString("Saved to %1").arg(saveFile));
}

void MainWindow::openDir() {
    auto newTempGameDir = QSharedPointer<QTemporaryDir>::create();
    auto newTempImportDir = QSharedPointer<QTemporaryDir>::create();
    if (!newTempGameDir->isValid() || !newTempImportDir->isValid()) {
        QMessageBox::critical(this, "Open Game Directory", "The temporary directory used for copying the game directory could not be created");
        return;
    }
    QString dirname = QFileDialog::getExistingDirectory(this, "Open Fortune Street Directory");
    if (dirname.isEmpty()) {
        return;
    }

    auto progress = QSharedPointer<QSharedPointer<CSMMProgressDialog>>::create(nullptr);

    auto copyTask = QtConcurrent::run([=]() {
        std::error_code error;
        std::filesystem::copy(dirname.toStdU16String(), newTempGameDir->path().toStdU16String(), std::filesystem::copy_options::recursive, error);
        return error;
    });
    if (!ImportExportUtils::isMainDolVanilla(QDir(dirname))) {
        auto btn = QMessageBox::warning(this, "Non-vanilla main.dol detected",
                             "CSMM has detected a non-vanilla main.dol; modifying a main.dol that has already been patched with CSMM is not fully supported. Continue anyway?",
                             QMessageBox::Yes | QMessageBox::No);
        if (btn != QMessageBox::Yes) {
            return;
        }
    }

    try {
        *progress = QSharedPointer<CSMMProgressDialog>::create("Importing folder", QString(), 0, 2, Qt::WindowFlags(), true);
        (*progress)->setWindowModality(Qt::WindowModal);
        (*progress)->setValue(0);

        auto gameInstance = GameInstance::fromGameDirectory(dirname, newTempImportDir->path());
        CSMMModpack modpack(gameInstance, modList.begin(), modList.end());
        modpack.load(dirname);
        (*progress)->checkCancel();
        (*progress)->setValue(1);
        auto errorCode = await(copyTask);
        (*progress)->checkCancel();
        (*progress)->setValue(2);
        if (errorCode) {
            QMessageBox::critical(this, "Error loading game", QString::fromStdString("Error copying files to temporary working directory: " + errorCode.message()));
            return;
        }
        loadDescriptors(gameInstance.mapDescriptors());
        setWindowFilePath(newTempGameDir->path());
        tempGameDir = newTempGameDir;
        importDir = newTempImportDir;
    } catch (const ProgressCanceled &) {
        // nothing to do
    } catch (const std::runtime_error &e) {
        *progress = nullptr;
        QMessageBox::critical(this, "Error loading game", QString("Error loading game: %1").arg(e.what()));
        PyErr_Clear();
    }
}

void MainWindow::openIsoWbfs() {
    auto newTempGameDir = QSharedPointer<QTemporaryDir>::create();
    auto newTempImportDir = QSharedPointer<QTemporaryDir>::create();
    if (!newTempGameDir->isValid() || !newTempImportDir->isValid()) {
        QMessageBox::critical(this, "Import WBFS/ISO", "The temporary directory used for importing disc images could not be created");
        return;
    }
    QString isoWbfs = QFileDialog::getOpenFileName(this, "Import WBFS/ISO", QString(), "Fortune Street disc files (*.wbfs *.iso *.ciso)");
    if (isoWbfs.isEmpty()) return;

    auto progress = QSharedPointer<QSharedPointer<CSMMProgressDialog>>::create(nullptr);

    *progress = QSharedPointer<CSMMProgressDialog>::create("Importing WBFS/ISO…", QString(), 0, 2, this, Qt::WindowFlags(), true);
    (*progress)->setWindowModality(Qt::WindowModal);
    (*progress)->setValue(0);
    try {
        await(ExeWrapper::extractWbfsIso(isoWbfs, newTempGameDir->path()));

        (*progress)->checkCancel();
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

        auto gameInstance = GameInstance::fromGameDirectory(dirname, newTempImportDir->path());
        CSMMModpack modpack(gameInstance, modList.begin(), modList.end());
        modpack.load(dirname);
        (*progress)->checkCancel();
        (*progress)->setValue(2);

        loadDescriptors(gameInstance.mapDescriptors());
        setWindowFilePath(newTempGameDir->path());
        tempGameDir = newTempGameDir;
        importDir = newTempImportDir;
    } catch (const ProgressCanceled &) {
        // nothing to do
    } catch (const std::runtime_error &e) {
        *progress = nullptr;
        QMessageBox::critical(this, "Error loading game", QString("Error loading game: %1").arg(e.what()));
        PyErr_Clear();
    }
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
    ui->actionExport_to_Riivolution->setEnabled(true);
}

void MainWindow::exportToFolder(bool riivolution) {
    auto saveDir = QFileDialog::getExistingDirectory(this, "Save to Fortune Street Directory");
    if (saveDir.isEmpty()) return;
    if (!QDir(saveDir).isEmpty()) {
        QMessageBox::critical(this, "Save", "Directory is non-empty");
        return;
    }
    // check if enough temporary disk space is available
    QTemporaryDir tmp;
    QStorageInfo storageInfo(tmp.path());
    int availableMb = storageInfo.bytesAvailable()/1024/1024;
    if(availableMb < 5000) {
        if (QMessageBox::question(this, "Save",
                              QString("There is less than 5 GB of space left on %1\nCSMM stores temporary files and needs enough disk space to function properly.").arg(storageInfo.displayName()),
                              QMessageBox::Ok|QMessageBox::Cancel) == QMessageBox::Cancel)
            return;
    }


    QString riivolutionName;
    if (riivolution) {
        bool riivolutionNameChosen;
        riivolutionName = QInputDialog::getText(this,
                                                "Enter Riivolution Patch Name",
                                                "Enter Riivolution Patch Name:",
                                                QLineEdit::Normal,
                                                "csmm", &riivolutionNameChosen);
        if (!riivolutionNameChosen) {
            return;
        }
        if (!Riivolution::validateRiivolutionName(riivolutionName)) {
            QMessageBox::critical(this, "Save", "Riivolution patch name is invalid");
            return;
        }
    }

    if (!riivolution && !ui->tableWidget->dirty) {
        if (QMessageBox::question(this, "Clean Export", "It seems you haven't made any changes.\nDo you want to make a clean export without letting CSMM make any game code changes and without applying any of the optional patches? ") == QMessageBox::Yes) {
            auto progress = QSharedPointer<CSMMProgressDialog>::create("Saving…", QString(), 0, 100);
            progress->setWindowModality(Qt::WindowModal);
            progress->setValue(0);

            // need to use utf 16 b/c windows behaves strangely w/ utf 8
            auto copyTask = QtConcurrent::run([=] {
                std::error_code error;
                std::filesystem::copy(windowFilePath().toStdU16String(), saveDir.toStdU16String(), std::filesystem::copy_options::recursive, error);
                return !error;
            });
            auto fut = AsyncFuture::observe(copyTask).subscribe([=](bool result) {
                if (!result) {
                    QMessageBox::critical(this, "Save", "Could not copy game data.");
                } else {
                    progress->setValue(100);
                    QMessageBox::information(this, "Save", "Saved successfuly.");
                }
            });
            return;
        }
    }

    if (std::find_if(modList.begin(), modList.end(), [](const CSMMModHolder &mod) {
                  return mod->modId() == "wifiFix";
})) {
        ui->statusbar->showMessage("Warning: Wiimmfi patching is not supported when exporting to a folder.");
    }

    auto wiiSaveDir = saveDir;
    if (riivolution) {
        if (!QDir(saveDir).mkdir(riivolutionName)) {
            QMessageBox::critical(this, "Export", "Export failed: Could not create riivolution directory");
        }
        wiiSaveDir = QDir(saveDir).filePath(riivolutionName);
    }

    auto progress = QSharedPointer<CSMMProgressDialog>::create("Saving…", QString(), 0, 100);
    progress->setWindowModality(Qt::WindowModal);
    progress->setValue(0);

    auto copyTask = QtConcurrent::run([=] {
        std::error_code error;
        std::filesystem::copy(windowFilePath().toStdU16String(), wiiSaveDir.toStdU16String(), std::filesystem::copy_options::recursive, error);
        return !error;
    });
    auto descriptors = QSharedPointer<std::vector<MapDescriptor>>::create();
    auto fut = AsyncFuture::observe(copyTask)
            .subscribe([=](bool result) {
        if (!result) {
            QMessageBox::critical(this, "Save", "Could not copy game data to temporary directory for modifying");
            return;
        }
        progress->setValue(30);
        auto descriptorPtrs = ui->tableWidget->getDescriptors();
        std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(*descriptors), [](auto &ptr) { return *ptr; });
        try {
            auto gameInstance = GameInstance::fromGameDirectory(wiiSaveDir, importDir->path(), *descriptors);
            CSMMModpack modpack(gameInstance, modList.begin(), modList.end());
            modpack.save(wiiSaveDir, [=](double progressVal) {
                progress->setValue(30 + (90 - 30) * progressVal);
            });

            if (riivolution) {
                progress->setValue(90);
                qInfo() << "Patching Riivolution…";
                Riivolution::write(windowFilePath(), saveDir, gameInstance.addressMapper(),
                                   riivolutionName);
            }

            progress->setValue(100);
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

    // check if enough temporary disk space is available
    QTemporaryDir tmp;
    QStorageInfo storageInfo(tmp.path());
    int availableMb = storageInfo.bytesAvailable()/1024/1024;
    if(availableMb < 5000) {
        if (QMessageBox::question(this, "Save",
                              QString("There is less than 5 GB of space left on %1\nCSMM stores temporary files and needs enough disk space to function properly.").arg(storageInfo.displayName()),
                              QMessageBox::Ok|QMessageBox::Cancel) == QMessageBox::Cancel)
            return;
    }

    if (!ui->tableWidget->dirty) {
        if (QMessageBox::question(this, "Clean Export", "It seems you haven't made any changes.\nDo you want to make a clean export without letting CSMM make any game code changes and without applying any of the optional patches?") == QMessageBox::Yes) {
            auto progress = QSharedPointer<CSMMProgressDialog>::create("Saving…", QString(), 0, 100);
            progress->setWindowModality(Qt::WindowModal);
            progress->setValue(0);
            auto fut = AsyncFuture::observe(ExeWrapper::createWbfsIso(windowFilePath(), saveFile, "01", false)).subscribe([=]() {
                progress->setValue(100);
                QMessageBox::information(this, "Export", "Exported successfuly.");
            });
            return;
        }
    }

    auto descriptors = QSharedPointer<std::vector<MapDescriptor>>::create();

    auto progress = QSharedPointer<CSMMProgressDialog>::create("Exporting to image…", QString(), 0, 100, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setValue(0);

    QString intermediatePath = intermediateResults->path();
    auto copyTask = QtConcurrent::run([=] {
        std::error_code error;
        std::filesystem::copy(windowFilePath().toStdU16String(), intermediatePath.toStdU16String(), std::filesystem::copy_options::recursive, error);
        return !error;
    });
    auto fut = AsyncFuture::observe(copyTask)
            .subscribe([=](bool result) {
        if (!result) {
            auto def = AsyncFuture::deferred<void>();
            def.complete();
            return def.future();
        }
        progress->setValue(20);
        auto descriptorPtrs = ui->tableWidget->getDescriptors();
        std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(*descriptors), [](auto &ptr) { return *ptr; });
        try {
            auto gameInstance = GameInstance::fromGameDirectory(intermediatePath, importDir->path(), *descriptors);
            CSMMModpack modpack(gameInstance, modList.begin(), modList.end());
            modpack.save(intermediatePath, [=](double progressVal) {
                progress->setValue(20 + (80 - 20) * progressVal);
            });
            *descriptors = gameInstance.mapDescriptors();
            progress->setValue(80);
            qInfo() << "packing wbfs/iso";
            return ExeWrapper::createWbfsIso(intermediatePath, saveFile, getMarkerCode(), getSeparateSaveGame());
        } catch (const std::runtime_error &exception) {
            QMessageBox::critical(this, "Export", QString("Export failed: %1").arg(exception.what()));
            PyErr_Clear();
            throw exception;
        }
    }).subscribe([=]() {
        progress->setValue(90);
        if (std::find_if(modList.begin(), modList.end(), [](const auto &mod) { return mod->modId() == "wifiFix"; })) {
            qInfo() << "patching wiimmfi";
            return ExeWrapper::patchWiimmfi(saveFile);
        }
        auto def = AsyncFuture::deferred<void>();
        def.complete();
        return def.future();
    }).subscribe([=]() {
        (void)intermediateResults; // keep temporary directory active while creating wbfs/iso

        progress->setValue(100);
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
        ValidationErrorDialog errorDialog(errorMsgs);
        errorDialog.exec();
    }
}

