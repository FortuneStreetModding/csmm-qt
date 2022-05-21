#include "uimessage.h"

#include <QTextStream>

UiMessage fileToMessage(QFile *file) {
    QTextStream stream(file);
    stream.setCodec("UTF-8");
    UiMessage result;
    QString line;
    while (stream.readLineInto(&line)) {
        int splitOn = line.indexOf(',');
        result[line.leftRef(splitOn).trimmed().toUInt()] = line.midRef(splitOn+1).trimmed().toString().replace("\"", "");
    }
    return result;
}

void messageToFile(QFile *file, const UiMessage &message) {
    QTextStream stream(file);
    stream.setCodec("UTF-8");
    for (auto it=message.begin(); it!=message.end(); ++it) {
        stream << QString("%1,\"%2\"\n").arg(it->first).arg(it->second);
    }
}
