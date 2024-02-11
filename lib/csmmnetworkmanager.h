#ifndef CSMMNETWORKMANAGER_H
#define CSMMNETWORKMANAGER_H

#include "qabstractnetworkcache.h"
#include <QFuture>
#include <QNetworkAccessManager>

namespace CSMMNetworkManager {
    long long int networkCacheSize();
    QString networkCacheDir();
    QNetworkAccessManager *instance();
    /**
     * @returns a future that resolves to whether the url is an internet url
     */
    bool downloadFileIfUrl(const QUrl &toDownloadFrom, const QString &dest,
                                    const std::function<void (double)> &progressCallback = [](double){});

    class Exception : public QException, public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
        const char *what() const noexcept override { return std::runtime_error::what(); }
        Exception(const QString &str) : std::runtime_error(str.toStdString()) {}
        void raise() const override { throw *this; }
        Exception *clone() const override { return new Exception(*this); }
    };

    inline void clearNetworkCache(){
        instance()->cache()->clear();
    }
}

#endif // CSMMNETWORKMANAGER_H
