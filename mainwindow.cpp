#include "mainwindow.h"
#include "additionalmodsdialog.h"
#include "lib/progresscanceled.h"
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
        if (QMessageBox::question(this, tr("Remove Map(s)"), tr("Are you sure you want to remove the selected maps?")) == QMessageBox::Yes) {
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
        auto saveFile = QFileDialog::getSaveFileName(this, tr("Export default modlist.txt"), "modlist.txt", "Mod List (*.txt)", nullptr);
        QSaveFile fileObj(saveFile);

        if (!fileObj.open(QFile::WriteOnly)) {
            QMessageBox::critical(this, tr("Error exporting modlist.txt"), tr("Error opening file for saving"));
        }

        auto defaultModList = DefaultModList::defaultModList();

        QTextStream stream(&fileObj);
        stream.setEncoding(QStringConverter::Utf8);

        for (auto &mod: defaultModList) {
            stream << mod->modId() << "\n";
        }

        fileObj.commit();
    });
    connect(ui->importModPack, &QPushButton::clicked, this, [&](bool) {
        AdditionalModsDialog *additionalModsDialog = new AdditionalModsDialog();

        if(additionalModsDialog->exec()){
            auto modpackZips = additionalModsDialog->GetModpackZips();
            if(modpackZips.isEmpty()){
                return;
            }
            try {
                QSettings settings;
                std::vector<std::shared_ptr<QTemporaryDir>> tmpDirs;

                for (int i = 0; i < modpackZips.size(); i++) {
                    tmpDirs.emplace_back(std::make_shared<QTemporaryDir>(ImportExportUtils::createTempDir("import")));
                }

                qInfo() << "Loading Modpacks: ";
                std::tie(modList, tempModpackDirs) = ModLoader::importModpackCollection(modpackZips, tmpDirs);
                updateModListWidget();
                QMessageBox::information(this, tr("Import modpack(s)"), tr("Modpack(s) successfully imported."));
            } catch (const std::runtime_error &error) {
                QMessageBox::critical(this, tr("Error importing modpack(s)"), QString(tr("Error importing modpack(s):\n%1")).arg(error.what()));
                PyErr_Clear();
            }
        }
    });
    connect(ui->quickSetup, &QPushButton::clicked, this, [&](bool) {
        auto dialog = new QuickSetupDialog(getMarkerCode(), getSeparateSaveGame());
        dialog->show();
        close();
    });
    connect(ui->actionPreferences, &QAction::triggered, this, [&]() {
        PreferencesDialog dialog;
        dialog.setWindowModality(Qt::ApplicationModal);

        connect(&dialog, &PreferencesDialog::territoryChangedSignal, this, [this](const QString& t) {
            qDebug() << "Territory: " + t;
            qDebug() << ui->tableWidget->getDescriptors().count();

            // if there are items in the MapDescriptorWidget,
            // reload them with the names and such from that territory.
            auto descriptors = ui->tableWidget->getDescriptors();
            if(ui->tableWidget->getDescriptors().count()!=0){
                ui->tableWidget->clearDescriptors();
                for (auto &d: descriptors) {
                    ui->tableWidget->appendMapDescriptor(*d);
                }
            }
        });

        dialog.exec();
    });

    updateModListWidget();
    setWindowTitle(QString("CSMM %1").arg(CSMM_VERSION));
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        qInfo() << "A MainWindow::changeEvent() has fired!";
        ui->retranslateUi(this);

        auto descriptors = ui->tableWidget->getDescriptors();
        if(ui->tableWidget->getDescriptors().count()!=0){
            ui->tableWidget->clearDescriptors();
            for (int i = 0; i < descriptors.count(); i++) {
                ui->tableWidget->appendMapDescriptor(*descriptors[i]);
                ui->tableWidget->loadRowWithMapDescriptor(i, *descriptors[i]);
            }
        }
        ui->tableWidget->retranslate();
    }

    QMainWindow::changeEvent(event);
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
    auto saveFile = QFileDialog::getSaveFileName(this, tr("Save Map List"), tr("mapList.yaml"), tr("CSMM Map List (*.yaml)"), nullptr);
    if (saveFile.isEmpty()) return;
    std::vector<MapDescriptor> descriptors;
    auto descriptorPtrs = ui->tableWidget->getDescriptors();
    std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(descriptors), [](auto &ptr) { return *ptr; });
    try {
        Configuration::save(saveFile, descriptors);
    } catch (const std::runtime_error &e) {
        QMessageBox::critical(this, tr("Save Map List"), QString(tr("Error saving the map list: %1")).arg(e.what()));
    }
}

void MainWindow::loadMapList() {
    auto openFile = QFileDialog::getOpenFileName(this, tr("Load Map List"), QString(), tr("CSMM Map List (*.yaml *.csv)"), nullptr);
    if (openFile.isEmpty()) return;
    QFileInfo openFileInfo(openFile);
    if(!openFileInfo.exists()) {
        QMessageBox::critical(this, tr("Open Map List"), QString(tr("Error loading the map list: %1")).arg(openFile));
    }
    std::vector<MapDescriptor> descriptors;
    auto descriptorPtrs = ui->tableWidget->getDescriptors();
    std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(descriptors), [](auto &ptr) { return *ptr; });
    ui->statusbar->showMessage(tr("Warning: This operation will import the maps in the map list one by one. Depending on the size of the map list, this can take a while and CSMM may freeze."));
    ui->statusbar->repaint();
    CSMMProgressDialog progressDialog(tr("Importing map list"), QString(), 0, 100);
    progressDialog.setWindowModality(Qt::ApplicationModal);
    try {
        Configuration::load(openFile, descriptors, importDir->path(), [&](double progress) {
            progressDialog.setValue(100 * progress);
        });
        loadDescriptors(descriptors);
        ui->tableWidget->dirty = true;
        ui->statusbar->showMessage(tr("Map list load completed"));
        ui->statusbar->repaint();
    } catch (const std::runtime_error &exception) {
        QMessageBox::critical(this, tr("Import Map List"), QString(tr("Error importing the map list: %1")).arg(exception.what()));
    }
}

void MainWindow::updateModListWidget() {
    ui->modListWidget->clear();
    QStringList modStrs;
    for (auto &mod: modList) {
        if(!modStrs.contains(mod->modId())){
            modStrs.append(mod->modId());
        }
        else{
            qInfo() << tr("Duplicate item attempted to be added to modList: ") + mod->modId();
        }
    }
    ui->modListWidget->addItems(modStrs);
}

void MainWindow::saveCleanItastCsmmBrsar() {
    auto openFile = QFileDialog::getOpenFileName(this, tr("Open vanilla Itast.brsar"), "Itast.brsar", tr("Vanilla Fortune Street Binary Sound Archive (*.brsar)"), nullptr);
    if (openFile.isEmpty()) return;
    //QFileInfo openFileInfo(openFile);

    if (ImportExportUtils::fileSha1(openFile) != ImportExportUtils::getSha1OfVanillaFileName(ITAST_BRSAR)) {
        QMessageBox::information(this, tr("Wrong Itast.brsar"), QString(tr("The provided file %1 is not a vanilla Itast.brsar")).arg(openFile));
        return;
    }

    auto saveFile = QFileDialog::getSaveFileName(this, tr("Save clean Itast.csmm.brsar"), "Itast.csmm.brsar", tr("CSMM Fortune Street Binary Sound Archive (*.brsar)"), nullptr);
    if (saveFile.isEmpty()) return;

    QString errors = ImportExportUtils::applyBspatch(openFile, saveFile, ":/" + ITAST_BRSAR + ".bsdiff");
    if(!errors.isEmpty()) {
        QMessageBox::critical(this, tr("Open"), QString(tr("Errors occurred when applying Itast.brsar.bsdiff patch to file %1:\n%2")).arg(openFile, errors));
    }

    QMessageBox::information(this, tr("Save"), QString(tr("Saved to %1")).arg(saveFile));
}

void MainWindow::openDir() {
    QSettings settings;
    auto newTempGameDir = QSharedPointer<QTemporaryDir>::create(ImportExportUtils::createTempDir("game"));
    auto newTempImportDir = QSharedPointer<QTemporaryDir>::create(ImportExportUtils::createTempDir("import"));
    if (!newTempGameDir->isValid() || !newTempImportDir->isValid()) {
        QMessageBox::critical(this, tr("Open Game Directory"), tr("The temporary directory used for copying the game directory could not be created"));
        return;
    }
    QString dirname = QFileDialog::getExistingDirectory(this, tr("Open Fortune Street Directory"), nullptr);
    if (dirname.isEmpty()) {
        return;
    }

    auto copyTask = QtConcurrent::run([=]() {
        std::error_code error;
        std::filesystem::copy(dirname.toStdU16String(), newTempGameDir->path().toStdU16String(), std::filesystem::copy_options::recursive, error);
        return error;
    });
    if (!ImportExportUtils::isMainDolVanilla(QDir(dirname))) {
        auto btn = QMessageBox::warning(this, tr("Non-vanilla main.dol detected"),
                                        tr("CSMM has detected a non-vanilla main.dol. Modifying a main.dol that has already been patched with CSMM is not fully supported. Continue anyway?"),
                             QMessageBox::Yes | QMessageBox::No);
        if (btn != QMessageBox::Yes) {
            return;
        }
    }

    try {
        CSMMProgressDialog progress(tr("Importing folder"), QString(), 0, 2, nullptr, Qt::WindowFlags(), true);
        progress.setWindowModality(Qt::WindowModal);
        progress.setValue(0);

        auto gameInstance = GameInstance::fromGameDirectory(dirname, newTempImportDir->path());
        CSMMModpack modpack(gameInstance, modList.begin(), modList.end());
        modpack.load(dirname);
        progress.setValue(1);
        auto errorCode = await(copyTask);
        progress.setValue(2);
        if (errorCode) {
            QMessageBox::critical(this, tr("Error loading game"), QString(tr("Error copying files to temporary working directory: %1")).arg(QString::fromStdString(errorCode.message())));
            return;
        }
        loadDescriptors(gameInstance.mapDescriptors());
        setWindowFilePath(newTempGameDir->path());
        tempGameDir = newTempGameDir;
        importDir = newTempImportDir;
    } catch (const ProgressCanceled &) {
        // nothing to do
    } catch (const std::runtime_error &e) {
        QMessageBox::critical(this, tr("Error loading game"), QString(tr("Error loading game: %1")).arg(e.what()));
        PyErr_Clear();
    }
}

void MainWindow::openIsoWbfs() {
    QSettings settings;
    auto newTempGameDir = QSharedPointer<QTemporaryDir>::create(ImportExportUtils::createTempDir("game"));
    auto newTempImportDir = QSharedPointer<QTemporaryDir>::create(ImportExportUtils::createTempDir("import"));
    if (!newTempGameDir->isValid() || !newTempImportDir->isValid()) {
        QMessageBox::critical(this, tr("Import WBFS/ISO"), tr("The temporary directory used for importing disc images could not be created"));
        return;
    }
    QString isoWbfs = QFileDialog::getOpenFileName(this, tr("Import WBFS/ISO"), QString(), tr("Fortune Street disc files (*.wbfs *.iso *.ciso)"), nullptr);
    if (isoWbfs.isEmpty()) return;

    try {
        CSMMProgressDialog progress(tr("Importing WBFS/ISO…"), QString(), 0, 2, nullptr, Qt::WindowFlags(), true);
        progress.setWindowModality(Qt::WindowModal);
        progress.setValue(0);

        await(ExeWrapper::extractWbfsIso(isoWbfs, newTempGameDir->path()));

        progress.setValue(1);

        if (!ImportExportUtils::isMainDolVanilla(newTempGameDir->path())) {
            auto btn = QMessageBox::warning(this, tr("Non-vanilla main.dol detected"),
                                            tr("CSMM has detected a non-vanilla main.dol. Modifying a main.dol that has already been patched with CSMM is not fully supported. Continue anyway?"),
                                 QMessageBox::Yes | QMessageBox::No);
            if (btn != QMessageBox::Yes) {
                return;
            }
        }
        auto dirname = newTempGameDir->path();

        auto gameInstance = GameInstance::fromGameDirectory(dirname, newTempImportDir->path());
        CSMMModpack modpack(gameInstance, modList.begin(), modList.end());
        modpack.load(dirname);
        progress.setValue(2);

        loadDescriptors(gameInstance.mapDescriptors());
        setWindowFilePath(newTempGameDir->path());
        tempGameDir = newTempGameDir;
        importDir = newTempImportDir;
    } catch (const ProgressCanceled &) {
        // nothing to do
    } catch (const std::runtime_error &e) {
        QMessageBox::critical(this, tr("Error loading game"), QString(tr("Error loading game: %1")).arg(e.what()));
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
    ui->importModPack->setEnabled(true);
    ui->actionValidate->setEnabled(true);
    ui->actionLoad_map_list_csv->setEnabled(true);
    ui->actionSave_map_list_csv->setEnabled(true);
    ui->actionExport_to_Folder->setEnabled(true);
    ui->actionExport_to_WBFS_ISO->setEnabled(true);
    ui->actionExport_to_Riivolution->setEnabled(true);
}

void MainWindow::exportToFolder(bool riivolution) {
    auto saveDir = QFileDialog::getExistingDirectory(this, tr("Save to Fortune Street Directory"), nullptr);
    if (saveDir.isEmpty()) return;
    if (!QDir(saveDir).isEmpty()) {
        QMessageBox::critical(this, tr("Save"), tr("Directory is not empty."));
        return;
    }
    // check if enough temporary disk space is available
    QSettings settings;
    QTemporaryDir tmp(ImportExportUtils::createTempDir("tmp"));
    QStorageInfo storageInfo(tmp.path());
    int availableMb = storageInfo.bytesAvailable()/1024/1024;
    if(availableMb < 10000) {
        if (QMessageBox::question(this, tr("Save"),
                                  QString(tr("%1 is configured to be your temporary directory, but the disk has less than 10GB of space available.\n\nIf you run out of disk space during the export operation, CSMM may crash. Would you like to proceed?")).arg(storageInfo.displayName()),
                              QMessageBox::Ok|QMessageBox::Cancel) == QMessageBox::Cancel)
            return;
    }


    QString riivolutionName;
    if (riivolution) {
        bool riivolutionNameChosen;
        riivolutionName = QInputDialog::getText(this,
                                                tr("Enter Riivolution Patch Name"),
                                                tr("Enter Riivolution Patch Name:"),
                                                QLineEdit::Normal,
                                                "csmm", &riivolutionNameChosen);
        if (!riivolutionNameChosen) {
            return;
        }
        if (!Riivolution::validateRiivolutionName(riivolutionName)) {
            QMessageBox::critical(this, tr("Save"), tr("Riivolution patch name is invalid"));
            return;
        }
    }

    if (!riivolution && !ui->tableWidget->dirty) {
        if (QMessageBox::question(this, tr("Clean Export"), tr("It seems you haven't made any changes.\nDo you want to make a clean export without letting CSMM make any game code changes and without applying any of the optional patches?")) == QMessageBox::Yes) {
            CSMMProgressDialog progress("Saving…", QString(), 0, 100, nullptr, Qt::WindowFlags(), true);
            try {
                progress.setWindowModality(Qt::WindowModal);
                progress.setValue(0);

                // need to use utf 16 b/c windows behaves strangely w/ utf 8
                std::error_code error;
                std::filesystem::copy(windowFilePath().toStdU16String(), saveDir.toStdU16String(), std::filesystem::copy_options::recursive, error);
                if (error) {
                    progress.close();
                    QMessageBox::critical(this, tr("Save"), QString(tr("Could not copy game data: %1")).arg(error.message().c_str()));
                } else {
                    progress.setValue(100);
                    QMessageBox::information(this, tr("Save"), tr("Saved successfuly."));
                }
            } catch (const ProgressCanceled &) { return; }
            return;
        }
    }

    if (std::find_if(modList.begin(), modList.end(), [](const CSMMModHolder &mod) {return mod->modId() == "wifiFix";}) != modList.end()) {
        ui->statusbar->showMessage(tr("Warning: Wiimmfi patching is not supported when exporting to a folder."));
    }



    auto wiiSaveDir = saveDir;
    if (riivolution) {
        if (!QDir(saveDir).mkdir(riivolutionName)) {
            QMessageBox::critical(this, tr("Export"), tr("Export failed: Could not create riivolution directory"));
        }
        wiiSaveDir = QDir(saveDir).filePath(riivolutionName);
    }

    CSMMProgressDialog progress(tr("Saving…"), QString(), 0, 100, nullptr, Qt::WindowFlags(), true);
    progress.setWindowModality(Qt::WindowModal);
    try {
        progress.setValue(0);

        std::error_code error;
        std::filesystem::copy(windowFilePath().toStdU16String(), wiiSaveDir.toStdU16String(), std::filesystem::copy_options::recursive, error);

        if (error) {
            QMessageBox::critical(this, tr("Save"), QString(tr("Could not copy game data to temporary directory for modifying:\n%1")).arg(error.message().c_str()));
            return;
        }

        progress.setValue(30);
        std::vector<MapDescriptor> descriptors;
        auto descriptorPtrs = ui->tableWidget->getDescriptors();
        std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(descriptors), [](auto &ptr) { return *ptr; });


        auto gameInstance = GameInstance::fromGameDirectory(wiiSaveDir, importDir->path(), descriptors);
        CSMMModpack modpack(gameInstance, modList.begin(), modList.end());
        modpack.save(wiiSaveDir, [&](double progressVal) {
            progress.setValue(30 + (90 - 30) * progressVal);
        });

        if (riivolution) {
            progress.setValue(90);
            qInfo() << tr("Patching Riivolution…");
            Riivolution::write(windowFilePath(), saveDir, gameInstance.addressMapper(),
                               riivolutionName);
        }

        qInfo() << tr("Cleaning up...");
        if(importDir->isValid()){
            importDir->remove();
        }

        if(tempGameDir->isValid()){
            tempGameDir->remove();
        }

        for (auto &d: tempModpackDirs) {
            if(d->isValid()){
                d->remove();
            }
        }

        progress.setValue(100);
        QMessageBox::information(this, tr("Save"), tr("Saved successfully."));

        // reload map descriptors
        int idx = 0;
        for (auto &descriptor: gameInstance.mapDescriptors()) {
            ui->tableWidget->loadRowWithMapDescriptor(idx++, descriptor);
        }
    } catch (const ProgressCanceled &) {
        return;
    } catch (const std::runtime_error &exception) {
        QMessageBox::critical(this, tr("Export"), QString(tr("Export failed: %1")).arg(exception.what()));
        PyErr_Clear();
    }
}

void MainWindow::exportIsoWbfs() {
    auto saveFile = QFileDialog::getSaveFileName(this, tr("Export WBFS/ISO"), QString(), tr("Fortune Street disc files (*.wbfs *.iso *.ciso)"), nullptr);
    if (saveFile.isEmpty()) return;
    QSettings settings;
    QTemporaryDir intermediateResults(ImportExportUtils::createTempDir("intermediate"));
    if (!intermediateResults.isValid()) {
        QMessageBox::critical(this, tr("Export WBFS/ISO"), tr("Could not create a temporary directory in the specified location. Please make sure the disk is mounted."));
        return;
    }

    // check if enough temporary disk space is available
    QTemporaryDir tmp(ImportExportUtils::createTempDir("tmp"));
    QStorageInfo storageInfo(tmp.path());
    int availableMb = storageInfo.bytesAvailable()/1024/1024;
    if(availableMb < 10000) {
        if (QMessageBox::question(this, tr("Save"),
                                  QString(tr("%1 is configured to be your temporary directory, but the disk has less than 10GB of space available.\n\nIf you run out of disk space during the export operation, CSMM may crash. Would you like to proceed?").arg(storageInfo.displayName())),
                              QMessageBox::Ok|QMessageBox::Cancel) == QMessageBox::Cancel)
            return;
    }

    if (!ui->tableWidget->dirty) {
        if (QMessageBox::question(this, tr("Clean Export"), tr("It seems you haven't made any changes.\nDo you want to make a clean export without letting CSMM make any game code changes and without applying any of the optional patches?")) == QMessageBox::Yes) {
            CSMMProgressDialog progress("Saving…", QString(), 0, 100, nullptr, Qt::WindowFlags(), true);
            progress.setWindowModality(Qt::WindowModal);

            try {
                progress.setValue(0);
                await(ExeWrapper::createWbfsIso(windowFilePath(), saveFile, "01", false));
                progress.setValue(100);
            } catch (const ProgressCanceled &) {
                return;
            } catch (const std::runtime_error &e) {
                QMessageBox::critical(this, tr("Export"), QString(tr("Export failed: %1")).arg(e.what()));
            }

            QMessageBox::information(this, tr("Export"), tr("Exported successfully."));
            return;
        }
    }

    std::vector<MapDescriptor> descriptors;
    CSMMProgressDialog progress(tr("Exporting to image…"), QString(), 0, 100, nullptr, Qt::WindowFlags(), true);
    progress.setWindowModality(Qt::WindowModal);

    try {
        progress.setValue(0);

        QString intermediatePath = intermediateResults.path();
        std::error_code error;
        std::filesystem::copy(windowFilePath().toStdU16String(), intermediatePath.toStdU16String(), std::filesystem::copy_options::recursive, error);
        if (error) {
            QMessageBox::critical(this, tr("Export"), QString(tr("Could not copy to intermediate directory: %1")).arg(error.message().c_str()));
        }

        progress.setValue(20);
        auto descriptorPtrs = ui->tableWidget->getDescriptors();
        std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(descriptors), [](auto &ptr) { return *ptr; });

        auto gameInstance = GameInstance::fromGameDirectory(intermediatePath, importDir->path(), descriptors);
        CSMMModpack modpack(gameInstance, modList.begin(), modList.end());
        modpack.save(intermediatePath, [&](double progressVal) {
            progress.setValue(20 + (80 - 20) * progressVal);
        });
        descriptors = gameInstance.mapDescriptors();

        progress.setValue(80);
        qInfo() << tr("Writing game image...");
        await(ExeWrapper::createWbfsIso(intermediatePath, saveFile, getMarkerCode(), getSeparateSaveGame()));

        progress.setValue(90);
        if (std::find_if(modList.begin(), modList.end(), [](const auto &mod) { return mod->modId() == "wifiFix"; }) != modList.end()) {
            qInfo() << tr("Patching Wiimmfi...");
            await(ExeWrapper::patchWiimmfi(saveFile));
        }

        progress.setValue(100);
        QMessageBox::information(this, tr("Export"), tr("Exported successfully."));

        // reload map descriptors
        int idx = 0;
        for (auto &descriptor: descriptors) {
            ui->tableWidget->loadRowWithMapDescriptor(idx++, descriptor);
        }
    } catch (const ProgressCanceled &) {
        return;
    } catch (const std::runtime_error &exception) {
        QMessageBox::critical(this, tr("Export"), QString(tr("Export failed: %1")).arg(exception.what()));
        PyErr_Clear();
    }
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

    for (int mapSet: std::as_const(mapSets)) {
        auto descriptorToZone = getMapZones(descriptors, mapSet, errorMsgs);
        auto zones = descriptorToZone.values();
        std::sort(zones.begin(), zones.end());
        zones.erase(std::unique(zones.begin(), zones.end()), zones.end());

        for (int zone: std::as_const(zones)) {
            getMapOrderings(descriptors, mapSet, zone, errorMsgs);
        }
    }

    if (errorMsgs.isEmpty()) {
        QMessageBox::information(this, tr("Validation"), tr("Validation passed."));
    } else {
        ValidationErrorDialog errorDialog(errorMsgs);
        errorDialog.exec();
    }
}

