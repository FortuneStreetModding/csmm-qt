#ifndef VANILLADATABASE_H
#define VANILLADATABASE_H

#include <QString>
#include "mapdescriptor.h"

namespace VanillaDatabase {
qint8 getVanillaMapSet(int mapId);
qint8 getVanillaZone(int mapId);
qint8 getVanillaOrder(int mapId);
const QString &getVentureCardDesc(int ventureCard);
QString getVanillaTpl(const QString &mapIcon);
bool hasVanillaTpl(const QString &mapIcon);
bool isVanillaBackground(const QString &background);
BgmId getDefaultBgmId(const QString &background);
bool hasDefaultBgmId(const QString &background);
QString getDefaultMapIcon(const QString &background);
bool hasDefaultMapIcon(const QString &background);
bool isDefaultVentureCards(const std::array<bool, 128> &ventureCards, RuleSet ruleSet);
void setDefaultVentureCards(RuleSet ruleSet, std::array<bool, 128> &outVentureCards);
QSet<QString> getVanillaIcons();
const QMap<QString, QString> &localeToDistrictWord();
const QMap<QString, std::vector<QString>> &getVanillaDistrictNames();
}

#endif // VANILLADATABASE_H
