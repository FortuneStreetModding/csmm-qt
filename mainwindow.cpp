#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <archive.h>
#include <archive_entry.h>
#include <QFileDialog>
#include <QMessageBox>
#include "lib/archiveutil.h"
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
    connect(ui->actionExport_To_Folder, &QAction::triggered, this, &MainWindow::exportToFolder);
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
    // TODO add *.dol
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
        ui->tableWidget->clearDescriptors();
        for (auto &descriptor: descriptors) {
            ui->tableWidget->appendMapDescriptor(descriptor);
        }
        setWindowFilePath(dirname);
        ui->actionExport_To_Folder->setEnabled(true);
    });
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
                return QFileInfo(file).fileName() == WIT_NAME ? appDir.filePath(WIT_NAME) : "";
            });
            auto downloadWszst = downloadRequiredFiles(dialog.getWszstURL(), [=](const QString &file) {
                auto filename = QFileInfo(file).fileName();
                if (filename == WSZST_NAME || filename == WIMGT_NAME) {
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
                    QMessageBox::critical(this, "Download", "Error occurred while downloading");
                    reply->deleteLater();
                    return false;
                }
                reply->deleteLater();

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
    QtShell::cp("-R", windowFilePath() + "/*", saveDir);
    auto descriptorPtrs = ui->tableWidget->getDescriptors();
    QVector<MapDescriptor> descriptors;
    std::transform(descriptorPtrs.begin(), descriptorPtrs.end(), std::back_inserter(descriptors), [&](auto &ptr) { return *ptr; });
    AsyncFuture::observe(PatchProcess::saveDir(saveDir, descriptors, false, ui->tableWidget->getTmpDir().path()))
            .subscribe([=](bool result) {
        if (result) {
            QMessageBox::information(this, "Save", "Saved successfuly.");
        } else {
            QMessageBox::critical(this, "Save", "An error occurred while saving.");
        }
    });

    // reload map descriptors
    int idx = 0;
    for (auto &descriptor: descriptors) {
        ui->tableWidget->loadRowWithMapDescriptor(idx++, descriptor);
    }
}
