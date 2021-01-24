#include "mainwindow.h"

#include "darkdetect.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    initDarkThemeSettings();

    MainWindow w;
    w.show();
    return a.exec();
}
