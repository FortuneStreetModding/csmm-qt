#include "csmmnetworkmanager.h"
#include <QDir>
#include <QSaveFile>
#include <QNetworkDiskCache>
#include <QApplication>
#include <QStandardPaths>
#include <QNetworkReply>
#include "lib/asyncfuture.h"
#include "lib/await.h"
#include "lib/progresscanceled.h"
#include <QtConcurrent>

namespace CSMMNetworkManager {

QNetworkAccessManager *instance() {
    static QNetworkAccessManager *theInstance = nullptr;
    if (!theInstance) {
        theInstance = new QNetworkAccessManager(QApplication::instance());
        auto diskCache = new QNetworkDiskCache(theInstance);
        diskCache->setCacheDirectory(networkCacheDir());
        diskCache->setMaximumCacheSize(4LL << 30); // 4 GB cache
        theInstance->setCache(diskCache);
        QObject::connect(theInstance, &QNetworkAccessManager::sslErrors, [=](QNetworkReply *reply, const QList<QSslError> &errors) {
            for (const QSslError &error : errors) {
                qWarning() << "SSL error:" << error.errorString();
            }
        });
    }
    return theInstance;
}

QString networkCacheDir() {
    QDir applicationCacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    return applicationCacheDir.filePath("networkCache");
}

bool downloadFileIfUrl(const QUrl &toDownloadFrom, const QString &dest,
                                const std::function<void(double)> &progressCallback) {
    if (!toDownloadFrom.isLocalFile()) {
        auto file = QSharedPointer<QSaveFile>::create(dest);
        if (file->open(QFile::WriteOnly)) {
            QNetworkRequest request(toDownloadFrom);
            request.setRawHeader("User-Agent", "CSMM (github.com/FortuneStreetModding/csmm-qt)");
            auto reply = instance()->get(request);
            QObject::connect(reply, &QNetworkReply::readyRead, instance(), [=]() {
                file->write(reply->readAll());
            });
            QObject::connect(reply, &QNetworkReply::downloadProgress, [=](qint64 elapsed, qint64 total) {
                try {
                    progressCallback(total == 0 ? 1 : (double)elapsed / total);
                } catch (const ProgressCanceled &) {
                    reply->abort();
                }
            });
            await(AsyncFuture::observe(reply, &QNetworkReply::finished).future());
            if (reply->error() != QNetworkReply::NoError) {
                auto errStr = reply->errorString();
                if (reply->error() == QNetworkReply::OperationCanceledError) {
                    delete reply;
                    throw ProgressCanceled("Canceled");
                }
                delete reply;
                QString fixSuggestion;
                if(errStr.contains("SSL handshake failed", Qt::CaseInsensitive)) {
                    fixSuggestion = "Check if your system time is set correctly and try again.";
                }
                throw Exception("network error: " + errStr + "\n" + fixSuggestion);
            }
            if (!file->commit()) {
                delete reply;
                throw Exception("write failed to " + dest);
            }
            delete reply;
            return true;
        } else {
            throw Exception("failed to create file for downloading");
        }
    }
    auto def = AsyncFuture::deferred<bool>();
    def.complete(false);
    return def.future().result();
}

}
