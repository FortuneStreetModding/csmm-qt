#ifndef DATAFILESET_H
#define DATAFILESET_H

#include "fslocale.h"

static const QString MAIN_DOL = "sys/main.dol";
static const QString PARAM_FOLDER = "files/param";
inline QString uiMessageCsv(const QString &locale) {
    return QString("files/localize/ui_message.%1.csv").arg(locale);
}
inline QString gameSequenceArc(const QString &locale) {
    if (locale == "jp") {
        return "files/game/game_sequence.arc";
    }
    return QString("files/game/lang%1/game_sequence_%1.arc").arg(localeToUpper(locale));
}
inline QString gameSequenceWifiArc(const QString &locale) {
    if (locale == "jp") {
        return "files/game/game_sequence_wifi.arc";
    }
    return QString("files/game/lang%1/game_sequence_wifi_%1.arc").arg(localeToUpper(locale));
}

#endif // DATAFILESET_H
