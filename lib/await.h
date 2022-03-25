#ifndef AWAIT_H
#define AWAIT_H

#include <QFuture>
#include <QEventLoop>
#include <QTimer>
#include <QFutureWatcher>

// From: https://github.com/benlau/asyncfuture/issues/11

template <typename T>
inline T await(QFuture<T> future, int timeout = -1) {
     if (future.isFinished()) {
         return future.result();
     }

     QFutureWatcher<T> watcher;
     watcher.setFuture(future);
     QEventLoop loop;

     if (timeout > 0) {
         QTimer::singleShot(timeout, &loop, &QEventLoop::quit);
     }

     QObject::connect(&watcher, SIGNAL(finished()), &loop, SLOT(quit()));
     loop.exec();

     return future.result();
}

inline void await(QFuture<void> future, int timeout = -1) {
     if (future.isFinished()) {
         return;
     }

     QFutureWatcher<void> watcher;
     watcher.setFuture(future);
     QEventLoop loop;

     if (timeout > 0) {
         QTimer::singleShot(timeout, &loop, &QEventLoop::quit);
     }

     QObject::connect(&watcher, SIGNAL(finished()), &loop, SLOT(quit()));
     loop.exec();

     future.waitForFinished(); // throw errors if applicable
}


#endif // AWAIT_H
