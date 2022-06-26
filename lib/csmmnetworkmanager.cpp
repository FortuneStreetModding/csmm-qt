#include "csmmnetworkmanager.h"
#include <QDir>
#include <QNetworkDiskCache>
#include <QApplication>
#include <QStandardPaths>

namespace CSMMNetworkManager {

static QNetworkAccessManager *theInstance = nullptr;

QNetworkAccessManager *instance() {
    if (!theInstance) {
        theInstance = new QNetworkAccessManager(QApplication::instance());
        auto diskCache = new QNetworkDiskCache(theInstance);
        diskCache->setCacheDirectory(networkCacheDir());
        diskCache->setMaximumCacheSize(4LL << 30); // 4 GB cache
        theInstance->setCache(diskCache);
    }
    return theInstance;
}

QString networkCacheDir() {
    QDir applicationCacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    return applicationCacheDir.filePath("networkCache");
}

}
