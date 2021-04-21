
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
        if (f.open(QIODevice::ReadOnly)) {
            QDataStream stream(&f);
            Brsar::File brsar;
            stream >> brsar;
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
