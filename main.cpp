#include "mainwindow.h"

#include <QApplication>
#include "lib/asyncfuture.h"
#include "lib/patchprocess.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#if 0
    AsyncFuture::observe(PatchProcess::openDir(QDir("/Volumes/ExtremeSSD/dolphin_stuff/fortune_street")))
            .subscribe([&](const QVector<MapDescriptor> &descriptors) {
        qDebug() << descriptors;
    });
#endif
    MainWindow w;
    w.show();
    return a.exec();
}
