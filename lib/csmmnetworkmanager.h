#ifndef CSMMNETWORKMANAGER_H
#define CSMMNETWORKMANAGER_H

#include <QNetworkAccessManager>

namespace CSMMNetworkManager {
    QString networkCacheDir();
    QNetworkAccessManager *instance();
}

#endif // CSMMNETWORKMANAGER_H
