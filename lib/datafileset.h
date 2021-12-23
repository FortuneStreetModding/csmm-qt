#ifndef DATAFILESET_H
#define DATAFILESET_H

#include "fslocale.h"

static const QString MAIN_DOL = "sys/main.dol";
static const QString CHANCE_CARD_FOLDER = "files/chance_card";
static const QString FONT_FOLDER = "files/font";
static const QString GAME_FOLDER = "files/game";
static const QString PARAM_FOLDER = "files/param";
static const QString SCENE_FOLDER = "files/scene";
static const QString SOUND_FOLDER = "files/sound";
static const QString SOUND_STREAM_FOLDER = "files/sound/stream";
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
inline QString bgPath(const QString &locale, const QString &background) {
    if (locale == "jp") {
        return QString("files/bg/%1.cmpres").arg(background);
    }
    return QString("files/bg/lang%1/%2_%1.cmpres").arg(localeToUpper(locale)).arg(background);
}
inline QString turnlotArc(const QString &background) {
    return GAME_FOLDER + "/game_turnlot_" + background + ".arc";
}
inline QString turnlotArcDir(const QString &background) {
    return GAME_FOLDER + "/game_turnlot_" + background;
}
inline QString turnlotPngFilename(const char extChr, const QString &background) {
    return "ui_menu_" + background + "_" + extChr + ".png";
}
inline QString turnlotPng(const char extChr, const QString &background) {
    return GAME_FOLDER + "/" + turnlotPngFilename(extChr, background);
}
inline QString turnlotTpl(const char extChr, const QString &background) {
    return turnlotArcDir(background) + "/arc/timg/ui_menu_" + background + extChr + ".tpl";
}
#endif // DATAFILESET_H
