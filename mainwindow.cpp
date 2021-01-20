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
#include <QtConcurrent>
#include "lib/asyncfuture.h"
#include "lib/exewrapper.h"
#include "lib/patchprocess.h"
#include "lib/qtshell/qtshell.h"
#include "downloadclidialog.h"

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
    connect(ui->actionCSMM_Help, &QAction::triggered, this, [&]() {
        QDesktopServices::openUrl(QUrl("https://github.com/FortuneStreetModding/csmm-qt/wiki"));
    });
    connect(ui->addMap, &QPushButton::clicked, this, [&](bool) { ui->tableWidget->appendMapDescriptor(MapDescriptor()); });
    connect(ui->removeMap, &QPushButton::clicked, this, [&](bool) {
        if (QMessageBox::question(this, "Remove Map(s)", "Are you sure you want to remove the selected maps?") == QMessageBox::Yes) {
            ui->tableWidget->removeSelectedMapDescriptors();
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFile() {
    QString dirname = QFileDialog::getExistingDirectory(this, "Open Fortune Street Directory");
    if (dirname.isEmpty()) {
        return;
    }
    AsyncFuture::observe(checkForRequiredFiles())
            .subscribe([=](bool result) {
        if (result) {
            return PatchProcess::openDir(QDir(dirname));
        }
        auto deferred = AsyncFuture::deferred<QVector<MapDescriptor>>();
        deferred.cancel();
        return deferred.future();
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
    QString isoWbfs = QFileDialog::getOpenFileName(this, "Import WBFS/ISO", QString(), "Fortune Street disc files (*.wbfs *.iso)");
    if (isoWbfs.isEmpty()) return;

    auto progress = QSharedPointer<QSharedPointer<QProgressDialog>>::create(nullptr);
    AsyncFuture::observe(checkForRequiredFiles())
            .subscribe([=](bool result) {
        *progress = QSharedPointer<QProgressDialog>::create("Importing WBFS/ISO…", QString(), 0, 2, this);
        if (result) {
            (*progress)->setWindowModality(Qt::WindowModal);
            (*progress)->setValue(0);
            return ExeWrapper::extractWbfsIso(isoWbfs, tempGameDir.path());
        }
        auto deferred = AsyncFuture::deferred<bool>();
        deferred.cancel();
        return deferred.future();
    }).subscribe([=](bool result) {
        if (!result) {
            QMessageBox::critical(this, "Import", "Error extracting WBFS/ISO");
            auto deferred = AsyncFuture::deferred<QVector<MapDescriptor>>();
            deferred.cancel();
            return deferred.future();
        }
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
    ui->mapToolbar->setEnabled(true);
    ui->actionExport_to_Folder->setEnabled(true);
    ui->actionExport_to_WBFS_ISO->setEnabled(true);
}

#ifdef Q_OS_WIN
#define WIT_URL "https://wit.wiimm.de/download/wit-v3.03a-r8245-cygwin.zip"
#define WSZST_URL "https://szs.wiimm.de/download/szs-v2.22a-r8323-cygwin32.zip"
#elif defined(Q_OS_MACOS)
#define WIT_URL "https://wit.wiimm.de/download/wit-v3.03a-r8245-mac.tar.gz"
#define WSZST_URL "https://szs.wiimm.de/download/szs-v2.22a-r8323-mac.tar.gz"
#else
#define WIT_URL "https://wit.wiimm.de/download/wit-v3.03a-r8245-x86_64.tar.gz"
#define WSZST_URL "https://szs.wiimm.de/download/szs-v2.22a-r8323-x86_64.tar.gz"
#endif

QFuture<bool> MainWindow::checkForRequiredFiles() {
    QDir appDir(QApplication::applicationDirPath());
    QString wit = appDir.filePath(WIT_NAME), wszst = appDir.filePath(WSZST_NAME), wimgt = appDir.filePath(WIMGT_NAME);
    QFileInfo witCheck(wit), wszstCheck(wszst), wimgtCheck(wimgt);
    if (!witCheck.exists() || !witCheck.isFile()
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
            return (AsyncFuture::combine() << downloadWit << downloadWszst).subscribe([=]() {
                bool result = downloadWit.result() && downloadWszst.result();
                if (result) {
                    QMessageBox::information(this, "Download", "Successfuly downloaded and extracted the programs.");
                } else {
                    QMessageBox::critical(this, "Download", "Error downloading or extracting the programs.");
                }
                return result;
            }).future();
        }
        auto def = AsyncFuture::deferred<bool>();
        def.complete(false);
        return def.future();
    }
    auto def = AsyncFuture::deferred<bool>();
    def.complete(true);
    return def.future();
}

template<class InToOutFiles>
QFuture<bool> MainWindow::downloadRequiredFiles(QUrl witURL, InToOutFiles func) {
    QSharedPointer<QTemporaryDir> tempDir(new QTemporaryDir);
    if (tempDir->isValid()) {
        auto witArchiveFile = new QFile(tempDir->filePath("temp_wit"), this);
        if (witArchiveFile->open(QIODevice::ReadWrite | QIODevice::Truncate)) {
            QNetworkReply *reply = manager->get(QNetworkRequest(witURL));
            connect(reply, &QNetworkReply::readyRead, this, [=]() {
                witArchiveFile->write(reply->readAll());
            });
            return AsyncFuture::observe(reply, &QNetworkReply::finished).subscribe([=]() -> bool {
                if (reply->error() != QNetworkReply::NoError) {
                    //QMessageBox::critical(this, "Download", "Error occurred while downloading");
                    reply->deleteLater();
                    return false;
                }
                reply->deleteLater();

                (void)tempDir; // keep alive

#ifdef Q_OS_WIN
                QString fname = witArchiveFile->fileName();
                witArchiveFile->close();

                zip_t *zip = zip_open(fname.toUtf8(), 0, 'r');
                //qDebug() << zip;
                int n = zip_total_entries(zip);
                //qDebug() << n;
                for (int i=0; i<n; ++i) {
                    zip_entry_openbyindex(zip, i);

                    auto name = zip_entry_name(zip);
                    QString output = func(name);
                    //qDebug() << name << output;
                    if (!zip_entry_isdir(zip) && !output.isEmpty()) {
                        zip_entry_fread(zip, output.toUtf8());
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
                        return false;
                    }
                } else {
                    return false;
                }
                archive_read_close(a);
                archive_read_free(a);
                archive_write_close(ext);
                archive_write_free(ext);
#endif
                witArchiveFile->deleteLater();

                return true;
            }).future();
        } else {
            auto def = AsyncFuture::deferred<bool>();
            def.complete(false);
            return def.future();
        }
    } else {
        auto def = AsyncFuture::deferred<bool>();
        def.complete(false);
        return def.future();
    }
}

void MainWindow::exportToFolder() {
    auto saveDir = QFileDialog::getExistingDirectory(this, "Save to Fortune Street Directory");
    if (saveDir.isEmpty()) return;
    if (!QDir(saveDir).isEmpty()) {
        QMessageBox::critical(this, "Save", "Directory is non-empty");
        return;
    }
    if (ui->actionPatch_Wiimmfi->isChecked()) {
        ui->statusbar->showMessage("Warning: Wiimmfi patching is not supported when exporting to a folder.");
    }

    auto progress = QSharedPointer<QProgressDialog>::create("Saving…", QString(), 0, 2, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setValue(0);

    auto copyTask = QtConcurrent::run([=] { return QtShell::cp("-R", windowFilePath() + "/*", saveDir); });
    auto descriptors = QSharedPointer<QVector<MapDescriptor>>::create();
    AsyncFuture::observe(copyTask)
            .subscribe([=](bool result) {
        if (!result) {
            auto def = AsyncFuture::deferred<bool>();
            def.complete(false);
            return def.future();
        }
        progress->setValue(1);
        auto descriptorPtrs = ui->tableWidget->getDescriptors();
        std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(*descriptors), [&](auto &ptr) { return *ptr; });
        return PatchProcess::saveDir(saveDir, *descriptors, false, ui->tableWidget->getTmpResourcesDir().path());
    }).subscribe([=](bool result) {
        progress->setValue(2);
        if (result) {
            QMessageBox::information(this, "Save", "Saved successfuly.");
        } else {
            QMessageBox::critical(this, "Save", "An error occurred while saving.");
        }

        // reload map descriptors
        int idx = 0;
        for (auto &descriptor: *descriptors) {
            ui->tableWidget->loadRowWithMapDescriptor(idx++, descriptor);
        }
    });
}

void MainWindow::exportIsoWbfs() {
    auto saveFile = QFileDialog::getSaveFileName(this, "Export WBFS/ISO", QString(), "Fortune Street disc files (*.wbfs *.iso)");
    if (saveFile.isEmpty()) return;

    auto intermediateResults = QSharedPointer<QTemporaryDir>::create();
    if (!intermediateResults->isValid()) {
        QMessageBox::critical(this, "Export WBFS/ISO", "Could not create a temporary directory for patching");
        return;
    }

    auto descriptors = QSharedPointer<QVector<MapDescriptor>>::create();

    auto progress = QSharedPointer<QProgressDialog>::create("Exporting to image…", QString(), 0, 3, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setValue(0);

    bool patchWiimmfi = ui->actionPatch_Wiimmfi->isChecked();
    auto copyTask = QtConcurrent::run([=] { return QtShell::cp("-R", windowFilePath() + "/*", intermediateResults->path()); });
    AsyncFuture::observe(copyTask)
            .subscribe([=](bool result) {
        if (!result) {
            auto def = AsyncFuture::deferred<bool>();
            def.complete(false);
            return def.future();
        }
        progress->setValue(1);
        auto descriptorPtrs = ui->tableWidget->getDescriptors();
        std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(*descriptors), [&](auto &ptr) { return *ptr; });
        return PatchProcess::saveDir(intermediateResults->path(), *descriptors, patchWiimmfi, ui->tableWidget->getTmpResourcesDir().path());
    }).subscribe([=](bool result) {
        if (!result) {
            auto def = AsyncFuture::deferred<bool>();
            def.complete(false);
            return def.future();
        }
        progress->setValue(2);
        return ExeWrapper::createWbfsIso(intermediateResults->path(), saveFile, patchWiimmfi);
    }).subscribe([=](bool result) {
        (void)intermediateResults; // keep temporary directory active while creating wbfs/iso

        progress->setValue(3);
        if (result) {
            QMessageBox::information(this, "Export", "Exported successfuly.");
        } else {
            QMessageBox::critical(this, "Export", "An error occurred while exporting.");
        }

        // reload map descriptors
        int idx = 0;
        for (auto &descriptor: *descriptors) {
            ui->tableWidget->loadRowWithMapDescriptor(idx++, descriptor);
        }
    });
}
