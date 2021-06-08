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
bool isDefaultVentureCards(const bool ventureCards[], RuleSet ruleSet);
void setDefaultVentureCards(RuleSet ruleSet, bool outVentureCards[]);
QSet<QString> getVanillaIcons();
}

#endif // VANILLADATABASE_H
