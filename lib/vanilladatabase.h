#ifndef VANILLADATABASE_H
#define VANILLADATABASE_H

#include <QString>

namespace VanillaDatabase {
qint8 getVanillaMapSet(int mapId);
qint8 getVanillaZone(int mapId);
qint8 getVanillaOrder(int mapId);
const QString &getVentureCardDesc(int ventureCard);
}

#endif // VANILLADATABASE_H
