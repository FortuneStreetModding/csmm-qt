#ifndef DOWNLOADTOOLS_H
#define DOWNLOADTOOLS_H

#include <QFuture>
#include <QNetworkAccessManager>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "lib/csmmnetworkmanager.h"
#include "lib/exewrapper.h"
#include "lib/asyncfuture.h"
#ifdef Q_OS_WIN
#include "lib/zip/zip.h"
#else
#include <archive.h>
#include <archive_entry.h>
#include "lib/archiveutil.h"
#endif

#ifdef Q_OS_WIN
#define WIT_URL "https://wit.wiimm.de/download/wit-v3.04a-r8427-cygwin32.zip"
#define WSZST_URL "https://szs.wiimm.de/download/szs-v2.27a-r8529-cygwin32.zip"
#elif defined(Q_OS_MACOS)
#define WIT_URL "https://wit.wiimm.de/download/wit-v3.04a-r8427-mac.tar.gz"
#define WSZST_URL "https://szs.wiimm.de/download/szs-v2.27a-r8529-mac.tar.gz"
#else
#define WIT_URL "https://wit.wiimm.de/download/wit-v3.04a-r8427-x86_64.tar.gz"
#define WSZST_URL "https://szs.wiimm.de/download/szs-v2.27a-r8529-x86_64.tar.gz"
#endif

namespace DownloadTools
{

    inline QDir getToolsLocation() {
        return QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    }

    class DownloadException : public QException, public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
        const char *what() const noexcept override { return std::runtime_error::what(); }
        DownloadException(const QString &str) : std::runtime_error(str.toStdString()) {}
        void raise() const override { throw *this; }
        DownloadException *clone() const override { return new DownloadException(*this); }
    };

    inline bool requiredFilesAvailable() {
        QDir appDir = getToolsLocation();
        QString wit = appDir.filePath(WIT_NAME), wszst = appDir.filePath(WSZST_NAME), wimgt = appDir.filePath(WIMGT_NAME);
        QFileInfo witCheck(wit), wszstCheck(wszst), wimgtCheck(wimgt);
        return witCheck.exists() && witCheck.isFile() &&
                wszstCheck.exists() && wszstCheck.isFile() &&
                wimgtCheck.exists() && wimgtCheck.isFile();
    }

    template<class ErrorCallback>
    inline QFuture<void> downloadAllRequiredFiles(ErrorCallback func, QString witUrl, QString wszstUrl) {
        QDir appDir = getToolsLocation();
        appDir.mkpath(".");
        auto downloadWit = downloadRequiredFiles(witUrl, [=](const QString &file) {
            auto filename = QFileInfo(file).fileName();
            if (filename == WIT_NAME || filename.endsWith(".dll")) {
                return appDir.filePath(filename);
            }
            return QString();
        });
        auto downloadWszst = downloadRequiredFiles(wszstUrl, [=](const QString &file) {
            auto filename = QFileInfo(file).fileName();
            if (filename == WSZST_NAME || filename == WIMGT_NAME || filename.endsWith(".dll")) {
                return appDir.filePath(filename);
            }
            return QString();
        });
        auto downloadFut = (AsyncFuture::combine() << downloadWit << downloadWszst);
        return downloadFut.subscribe([=]() {}, [=]() {
            try {
                auto _downloadWit = downloadWit;
                auto _downloadWszst = downloadWszst;
                _downloadWit.waitForFinished();
                _downloadWszst.waitForFinished();
            } catch (const DownloadException &ex) {
                func(ex.what());
            }
        }).future();
    }

    template<class InToOutFiles>
    inline QFuture<void> downloadRequiredFiles(QUrl url, InToOutFiles func) {
        auto manager = CSMMNetworkManager::instance();
        QSharedPointer<QTemporaryDir> tempDir(new QTemporaryDir);
        if (tempDir->isValid()) {
            auto witArchiveFile = QSharedPointer<QFile>::create(tempDir->filePath("temp_wit"));
            if (witArchiveFile->open(QIODevice::ReadWrite | QIODevice::Truncate)) {
                QNetworkRequest request(url);
                request.setRawHeader("User-Agent", "CSMM (github.com/FortuneStreetModding/csmm-qt)");
                QNetworkReply *reply = manager->get(request);
                //manager->setTransferTimeout(1000);
                QObject::connect(reply, &QNetworkReply::readyRead, manager, [=]() {
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
                }).future();
            } else {
                throw DownloadException(QString("Temporary file could not be created"));
            }
        } else {
            throw DownloadException(QString("Temporary directory could not be created"));
        }
    }

};

#endif // DOWNLOADTOOLS_H
