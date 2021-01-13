#ifndef EXEWRAPPER_H
#define EXEWRAPPER_H

#include <QFuture>
#include "addressmapping.h"

#ifdef Q_OS_WIN
#define WIT_NAME "wit.exe"
#else
#define WIT_NAME "wit"
#endif

namespace ExeWrapper {
    QFuture<QVector<AddressSection>> readSections(const QString &inputFile);
}

#endif // EXEWRAPPER_H
