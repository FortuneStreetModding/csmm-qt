#ifndef UIMESSAGE_H
#define UIMESSAGE_H

#include <QFile>

typedef std::map<quint32, QString> UiMessage;

UiMessage fileToMessage(QFile *file);
void messageToFile(QFile *file, const UiMessage &message);

#endif // UIMESSAGE_H
