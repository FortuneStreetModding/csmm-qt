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
#include "downloadclidialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), manager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFile() {
    // todo add *.dol
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
        for (auto &descriptor: descriptors) {
            ui->tableWidget->appendMapDescriptor(descriptor);
        }
    });
}

#ifdef Q_OS_WIN
#define WIT_URL "https://wit.wiimm.de/download/wit-v3.03a-r8245-cygwin.zip"
#elif defined(Q_OS_MACOS)
#define WIT_URL "https://wit.wiimm.de/download/wit-v3.03a-r8245-mac.tar.gz"
#else
#define WIT_URL "https://wit.wiimm.de/download/wit-v3.03a-r8245-x86_64.tar.gz"
#endif

QFuture<bool> MainWindow::checkForRequiredFiles() {
    QDir appDir(QApplication::applicationDirPath());
    QString wit = appDir.filePath(WIT_NAME);
    QFileInfo witCheck(wit);
    if (!witCheck.exists() || !witCheck.isFile()) {
        DownloadCLIDialog dialog(WIT_URL, this);
        if (dialog.exec() == QDialog::Accepted) {
            return AsyncFuture::observe(downloadRequiredFiles(dialog.getWitURL(), [=](const QString &file) {
                return QFileInfo(file).fileName() == WIT_NAME ? appDir.filePath(WIT_NAME) : "";
            })).subscribe([&](bool result) {
                if (result) {
                    QMessageBox::information(this, "Download", "Successfuly downloaded and extracted Wit.");
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
        auto witArchiveFile = new QFile(tempDir->filePath("temp"), this);
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
                        QMessageBox::critical(this, "Download", "Error reading archive.");
                        return false;
                    }
                } else {
                    QMessageBox::critical(this, "Download", QString("Error opening archive: %1").arg(archive_error_string(a)));
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
            QMessageBox::critical(this, "Download", "Error opening file for download");
            auto def = AsyncFuture::deferred<bool>();
            def.complete(false);
            return def.future();
        }
    } else {
        QMessageBox::critical(this, "Download", "Error creating temporary directory");
        auto def = AsyncFuture::deferred<bool>();
        def.complete(false);
        return def.future();
    }
}
