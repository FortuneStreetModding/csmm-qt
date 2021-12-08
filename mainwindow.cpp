#include "mainwindow.h"
#include "ui_mainwindow.h"

#ifdef Q_OS_WIN
#include "lib/zip/zip.h"
#else
#include <archive.h>
#include <archive_entry.h>
#include "lib/archiveutil.h"
#endif
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QInputDialog>
#include "lib/asyncfuture.h"
#include "lib/exewrapper.h"
#include "lib/patchprocess.h"
#include "lib/qtshell/qtshell.h"
#include "downloadclidialog.h"
#include "validationerrordialog.h"
#include "lib/configuration.h"
#include "lib/datafileset.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), manager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    ui->tableWidget->setGameDirectoryFunction([&]() { return windowFilePath(); });
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
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
    QVector<MapDescriptor> descriptors;
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
    QVector<MapDescriptor> descriptors;
    auto descriptorPtrs = ui->tableWidget->getDescriptors();
    std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(descriptors), [](auto &ptr) { return *ptr; });
    ui->statusbar->showMessage("Warning: This operation will import the maps in the map list one by one. Depending on the size of the map list, this can take a while and CSMM may freeze.");
    ui->statusbar->repaint();
    try {
        Configuration::load(openFile, descriptors, ui->tableWidget->getTmpResourcesDir().path());
        loadDescriptors(descriptors);
        ui->tableWidget->dirty = true;
        ui->statusbar->showMessage("Map list load completed");
        ui->statusbar->repaint();
    } catch (const PatchProcess::Exception &exception) {
        QMessageBox::critical(this, "Import .yaml", QString("Error loading the map: %1").arg(exception.getMessage()));
    } catch (const YAML::Exception &exception) {
        QMessageBox::critical(this, "Import .yaml", QString("Error loading the map: %1").arg(exception.what()));
    }
}

void MainWindow::saveCleanItastCsmmBrsar() {
    auto openFile = QFileDialog::getOpenFileName(this, "Open vanilla Itast.brsar", "Itast.brsar", "Vanilla Fortune Street Binary Sound Archive (*.brsar)");
    if (openFile.isEmpty()) return;
    QFileInfo openFileInfo(openFile);

    if (PatchProcess::fileSha1(openFile) != PatchProcess::getSha1OfVanillaFileName("sound/Itast.brsar")) {
        QMessageBox::information(this, "Wrong Itast.brsar", QString("The provided file %1 is not a vanilla Itast.brsar").arg(openFile));
        return;
    }

    auto saveFile = QFileDialog::getSaveFileName(this, "Save clean Itast.csmm.brsar", "Itast.csmm.brsar", "CSMM Fortune Street Binary Sound Archive (*.brsar)");
    if (saveFile.isEmpty()) return;

    QString errors = PatchProcess::applyBspatch(openFile, saveFile, ":/" + SOUND_FOLDER + "/Itast.brsar.bsdiff");
    if(!errors.isEmpty()) {
        QMessageBox::critical(this, "Open", QString("Errors occurred when applying Itast.brsar.bsdiff patch to file %1:\n%2").arg(openFile).arg(errors));
    }

    QMessageBox::information(this, "Save", QString("Saved to %1").arg(saveFile));
}

void MainWindow::openFile() {
    QString dirname = QFileDialog::getExistingDirectory(this, "Open Fortune Street Directory");
    if (dirname.isEmpty()) {
        return;
    }
    AsyncFuture::observe(checkForRequiredFiles())
            .subscribe([=]() {
        return PatchProcess::openDir(QDir(dirname));
    }).subscribe([=](const QVector<MapDescriptor> &descriptors) {
        if (descriptors.isEmpty()) {
            QMessageBox::critical(this, "Open", "Bad Fortune Street directory");
            return;
        }
        loadDescriptors(descriptors);
        setWindowFilePath(dirname);
    });
}

void MainWindow::openIsoWbfs() {
    if (!tempGameDir.isValid()) {
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
        return ExeWrapper::extractWbfsIso(isoWbfs, tempGameDir.path());
    }).subscribe([=]() {
        (*progress)->setValue(1);
        return PatchProcess::openDir(tempGameDir.path());
    }).subscribe([=](const QVector<MapDescriptor> &descriptors) {
        if (descriptors.isEmpty()) {
            QMessageBox::critical(this, "Import", "Error loading WBFS/ISO file contents");
            return;
        }
        (*progress)->setValue(2);
        loadDescriptors(descriptors);
        setWindowFilePath(tempGameDir.path());
    });
}

void MainWindow::loadDescriptors(const QVector<MapDescriptor> &descriptors) {
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

#ifdef Q_OS_WIN
#define WIT_URL "https://wit.wiimm.de/download/wit-v3.04a-r8427-cygwin32.zip"
#define WSZST_URL "https://szs.wiimm.de/download/szs-v2.22a-r8323-cygwin32.zip"
#elif defined(Q_OS_MACOS)
#define WIT_URL "https://wit.wiimm.de/download/wit-v3.04a-r8427-mac.tar.gz"
#define WSZST_URL "https://szs.wiimm.de/download/szs-v2.22a-r8323-mac.tar.gz"
#else
#define WIT_URL "https://wit.wiimm.de/download/wit-v3.04a-r8427-x86_64.tar.gz"
#define WSZST_URL "https://szs.wiimm.de/download/szs-v2.22a-r8323-x86_64.tar.gz"
#endif

namespace {
    class DownloadException : public QException {
    public:
        DownloadException(const QString &msgStr) : message(msgStr) {}
        const QString &getMessage() const { return message; }
        void raise() const override { throw *this; }
        DownloadException *clone() const override { return new DownloadException(*this); }
    private:
        QString message;
    };
}

QFuture<void> MainWindow::checkForRequiredFiles(bool alwaysAsk) {
    QDir appDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    appDir.mkpath(".");
    QString wit = appDir.filePath(WIT_NAME), wszst = appDir.filePath(WSZST_NAME), wimgt = appDir.filePath(WIMGT_NAME);
    QFileInfo witCheck(wit), wszstCheck(wszst), wimgtCheck(wimgt);
    if (alwaysAsk || !witCheck.exists() || !witCheck.isFile()
            || !wszstCheck.exists() || !wszstCheck.isFile()
            || !wimgtCheck.exists() || !wimgtCheck.isFile()) {
        DownloadCLIDialog dialog(WIT_URL, WSZST_URL, this);
        if (dialog.exec() == QDialog::Accepted) {
            auto downloadWit = downloadRequiredFiles(dialog.getWitURL(), [=](const QString &file) {
                auto filename = QFileInfo(file).fileName();
                if (filename == WIT_NAME || filename.endsWith(".dll")) {
                    return appDir.filePath(filename);
                }
                return QString();
            });
            auto downloadWszst = downloadRequiredFiles(dialog.getWszstURL(), [=](const QString &file) {
                auto filename = QFileInfo(file).fileName();
                if (filename == WSZST_NAME || filename == WIMGT_NAME || filename.endsWith(".dll")) {
                    return appDir.filePath(filename);
                }
                return QString();
            });
            auto downloadFut = (AsyncFuture::combine() << downloadWit << downloadWszst);
            return downloadFut.subscribe([=]() {
                QMessageBox::information(this, "Download", "Successfuly downloaded and extracted the programs.");
            }, [=]() {
                try {
                    auto _downloadWit = downloadWit;
                    auto _downloadWszst = downloadWszst;
                    _downloadWit.waitForFinished();
                    _downloadWszst.waitForFinished();
                } catch (const DownloadException &ex) {
                    QMessageBox::critical(this, "Download", ex.getMessage());
                }
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

template<class InToOutFiles>
QFuture<void> MainWindow::downloadRequiredFiles(QUrl witURL, InToOutFiles func) {
    QSharedPointer<QTemporaryDir> tempDir(new QTemporaryDir);
    if (tempDir->isValid()) {
        auto witArchiveFile = new QFile(tempDir->filePath("temp_wit"), this);
        if (witArchiveFile->open(QIODevice::ReadWrite | QIODevice::Truncate)) {
            QNetworkRequest request = QNetworkRequest(witURL);
            request.setRawHeader("User-Agent", "CSMM (github.com/FortuneStreetModding/csmm-qt)");
            QNetworkReply *reply = manager->get(request);
            //manager->setTransferTimeout(1000);
            connect(reply, &QNetworkReply::readyRead, this, [=]() {
                witArchiveFile->write(reply->readAll());
            });
            return AsyncFuture::observe(reply, &QNetworkReply::finished).subscribe([=]() -> void {
                if (reply->error() != QNetworkReply::NoError) {
                    DownloadException ex(QString("Error downloading: %1").arg(reply->errorString()));
                    reply->deleteLater();
                    throw ex;
                }
                reply->deleteLater();

                (void)tempDir; // keep alive

#ifdef Q_OS_WIN
                QString fname = witArchiveFile->fileName();
                witArchiveFile->close();

                zip_t *zip = zip_open(fname.toUtf8(), 0, 'r');
                if (!zip) {
                    throw DownloadException(QString("Error opening downloaded file %1").arg(fname));
                }
                int n = zip_total_entries(zip);
                for (int i=0; i<n; ++i) {
                    zip_entry_openbyindex(zip, i);

                    auto name = zip_entry_name(zip);
                    QString output = func(name);
                    if (!zip_entry_isdir(zip) && !output.isEmpty()) {
                        if (zip_entry_fread(zip, output.toUtf8()) < 0) {
                            zip_close(zip);
                            throw DownloadException(QString("Error extracting downloaded file %1").arg(fname));
                        }
                    }

                    zip_entry_close(zip);
                }
                zip_close(zip);
#else
                int flags = ARCHIVE_EXTRACT_TIME;
                flags |= ARCHIVE_EXTRACT_PERM;
                flags |= ARCHIVE_EXTRACT_ACL;
                flags |= ARCHIVE_EXTRACT_FFLAGS;

                archive *a = archive_read_new();
                archive_read_support_format_all(a);
                archive_read_support_filter_all(a);
                archive *ext = archive_write_disk_new();
                archive_write_disk_set_options(ext, flags);
                archive_write_disk_set_standard_lookup(ext);
                witArchiveFile->seek(0);
                int r = archive_read_open_fd(a, witArchiveFile->handle(), 16384);
                if (r == ARCHIVE_OK) {
                    r = extractFileTo(a, ext, func);
                    if (r != ARCHIVE_OK) {
                        throw DownloadException(QString("Error extracting downloaded file %1").arg(witArchiveFile->fileName()));
                    }
                } else {
                    throw DownloadException(QString("Error opening downloaded file %1").arg(witArchiveFile->fileName()));
                }
                archive_read_close(a);
                archive_read_free(a);
                archive_write_close(ext);
                archive_write_free(ext);
#endif
                witArchiveFile->deleteLater();
            }).future();
        } else {
            throw DownloadException(QString("Temporary file could not be created"));
        }
    } else {
        throw DownloadException(QString("Temporary directory could not be created"));
    }
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

            auto copyTask = QtConcurrent::run([=] { return QtShell::cp("-R", windowFilePath() + "/*", saveDir); });
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

    auto copyTask = QtConcurrent::run([=] { return QtShell::cp("-R", windowFilePath() + "/*", saveDir); });
    auto descriptors = QSharedPointer<QVector<MapDescriptor>>::create();
    auto fut = AsyncFuture::observe(copyTask)
            .subscribe([=](bool result) {
        if (!result) {
            QMessageBox::critical(this, "Save", "Could not copy game data to temporary directory for modifying");
            auto def = AsyncFuture::deferred<void>();
            def.cancel();
            return def.future();
        }
        progress->setValue(1);
        auto descriptorPtrs = ui->tableWidget->getDescriptors();
        std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(*descriptors), [](auto &ptr) { return *ptr; });
        try {
            return PatchProcess::saveDir(saveDir, *descriptors, false, ui->tableWidget->getTmpResourcesDir().path());
        } catch (const PatchProcess::Exception &exception) {
            QMessageBox::critical(this, "Export", QString("Export failed: %1").arg(exception.getMessage()));
            throw exception;
        }
    }).subscribe([=]() {
        progress->setValue(2);
        QMessageBox::information(this, "Save", "Saved successfuly.");

        // reload map descriptors
        int idx = 0;
        for (auto &descriptor: *descriptors) {
            ui->tableWidget->loadRowWithMapDescriptor(idx++, descriptor);
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

    auto descriptors = QSharedPointer<QVector<MapDescriptor>>::create();

    auto progress = QSharedPointer<QProgressDialog>::create("Exporting to image…", QString(), 0, 4, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setValue(0);

    bool patchWiimmfi = ui->actionPatch_Wiimmfi->isChecked();
    QString intermediatePath = intermediateResults->path();
    auto copyTask = QtConcurrent::run([=] { return QtShell::cp("-R", windowFilePath() + "/*", intermediatePath); });
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
            return PatchProcess::saveDir(intermediatePath, *descriptors, patchWiimmfi, ui->tableWidget->getTmpResourcesDir().path());
        } catch (const PatchProcess::Exception &exception) {
            QMessageBox::critical(this, "Export", QString("Export failed: %1").arg(exception.getMessage()));
            throw exception;
        }
    }).subscribe([=]() {
        progress->setValue(2);
        return ExeWrapper::createWbfsIso(intermediatePath, saveFile, getSaveId());
    }).subscribe([=]() {
        progress->setValue(3);
        if (patchWiimmfi)
            return ExeWrapper::patchWiimmfi(saveFile);
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
    QVector<MapDescriptor> descriptors;
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
