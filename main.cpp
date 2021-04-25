
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
        path.append("/Itast.brsar");
        QFile f(path);
        Brsar::File brsar;
        if (f.open(QIODevice::ReadOnly)) {
            QDataStream stream(&f);
            stream >> brsar;
            f.close();
        }

        path = qApp->applicationDirPath();
        path.append("/Itast2.brsar");
        QFile f2(path);
        if (f2.open(QIODevice::WriteOnly)) {
            QDataStream stream(&f2);
            stream << brsar;
            f2.close();
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
