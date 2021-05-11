
/*
#include "mainwindow.h"
#include "darkdetect.h"
#include <QApplication>*/

#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QTimer>
#include "lib/brsar/brsar.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QTimer::singleShot(0,[](){
        QString path = qApp->applicationDirPath();
        path.append("/itast.csmm.brsar");
        QFile orig(path);
        path = qApp->applicationDirPath();
        path.append("/itast.csmm.patched.brsar");
        if (QFile::exists(path))
        {
            QFile::remove(path);
        }
        orig.copy(path);
        QFile f(path);
        if (f.open(QIODevice::ReadWrite)) {
            QDataStream stream(&f);
            QVector<MapDescriptor> mapDescriptors;
            Brsar::patch(stream, mapDescriptors);
            f.close();
        }
        qDebug() << "Finish";
        QCoreApplication::exit(0);

    });



    return a.exec();

/*
    QApplication a(argc, argv);
    initDarkThemeSettings();

    MainWindow w;
    w.show();

    return a.exec();*/
}
