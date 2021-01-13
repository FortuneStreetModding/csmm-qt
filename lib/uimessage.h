#ifndef UIMESSAGE_H
#define UIMESSAGE_H

#include <QMap>
#include <QFile>

typedef QMap<quint32, QString> UiMessage;

UiMessage fileToMessage(QFile *file);
void messageToFile(QFile *file, const UiMessage &message);
quint32 freeKey(const UiMessage &message);

#endif // UIMESSAGE_H
