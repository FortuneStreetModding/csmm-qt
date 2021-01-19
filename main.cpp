#include "mainwindow.h"

#include <QApplication>
#include "lib/asyncfuture.h"
#include "lib/patchprocess.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
