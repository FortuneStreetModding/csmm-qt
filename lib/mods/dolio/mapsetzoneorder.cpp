#include "mapsetzoneorder.h"
#include "lib/powerpcasm.h"
#include "lib/vanilladatabase.h"

quint32 MapSetZoneOrder::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QByteArray mapSetZoneOrderTable;
    for (auto &descriptor: descriptors) {
        mapSetZoneOrderTable.append(descriptor.mapSet);
        qint8 zone = descriptor.zone;
        if (zone == 0) {
            zone = 1;
        } else if (zone == 1) {
            zone = 0;
        }
        mapSetZoneOrderTable.append(zone);
        mapSetZoneOrderTable.append(descriptor.order);
    }
    return allocate(mapSetZoneOrderTable, "MapSetZoneOrderTable");
}

void MapSetZoneOrder::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);

    // --- Game::GameSequenceDataAdapter::GetNumMapsInZone ---
    quint32 subroutineGetNumMapsInZone = allocate(writeSubroutineGetNumMapsInZone(mapDescriptors), "SubroutineGetNumMapsInZone");
    quint32 hijackAddr = addressMapper.boomStreetToStandard(0x8020f3ac);
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // li r3,0x6 ->  b subroutineGetNumMapsInZone
    stream << PowerPcAsm::b(hijackAddr, subroutineGetNumMapsInZone);

    // --- Game::GameSequenceDataAdapter::GetMapsInZone ---
    hijackAddr = addressMapper.boomStreetToStandard(0x8020f454);
    quint32 returnAddr = addressMapper.boomStreetToStandard(0x8020f554);
    quint32 subroutineGetMapsInZone = allocate(writeSubroutineGetMapsInZone(addressMapper, mapDescriptors, tableAddr, 0, returnAddr), "SubroutineGetMapsInZone");
    stream.device()->seek(addressMapper.toFileAddress(subroutineGetMapsInZone));
    auto insts = writeSubroutineGetMapsInZone(addressMapper, mapDescriptors, tableAddr, subroutineGetMapsInZone, returnAddr); // re-write the routine again since now we know where it is located in the main dol
    for (quint32 inst: qAsConst(insts)) stream << inst;
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // cmpwi r29,0x0 ->  b subroutineGetMapsInZone
    stream << PowerPcAsm::b(hijackAddr, subroutineGetMapsInZone);

    // --- Write Table Meta Information ---
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020f458));
    stream << (uint)tableAddr;
    stream << (short)0;
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020F45E));
    stream << (short)mapDescriptors.size();
}

void MapSetZoneOrder::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        setVanillaMapSetZoneOrder(mapDescriptors);
    } else {
        for (auto &mapDescriptor: mapDescriptors) {
            stream >> mapDescriptor.mapSet;
            stream >> mapDescriptor.zone;
            if (mapDescriptor.zone == 0) {
                mapDescriptor.zone = 1;
            } else if (mapDescriptor.zone == 1) {
                mapDescriptor.zone = 0;
            }
            stream >> mapDescriptor.order;
        }
    }
}

quint32 MapSetZoneOrder::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) {
    if (isVanilla) {
        return 0;
    }
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020f458));
    quint32 addr; stream >> addr;
    return addr;
}

qint16 MapSetZoneOrder::readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) {
    if (isVanilla) return -1;
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020F45E));
    qint16 result; stream >> result;
    return result;
}

bool MapSetZoneOrder::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x8020f454));
    quint32 opcode; stream >> opcode;
    // mulli r0,r3,0x38
    return opcode == PowerPcAsm::cmpwi(29, 0);
}

QVector<quint32> MapSetZoneOrder::writeSubroutineGetNumMapsInZone(const std::vector<MapDescriptor> &mapDescriptors) {
    // precondition:  r3  _ZONE_TYPE
    // postcondition: r3  num maps
    QVector<quint32> asm_, asm_l2;

    // load current mapSet into r4
    asm_.append(PowerPcAsm::lwz(4, 0xa7fc, 13));    // GameInfo (0x8081747c) -> r4
    asm_.append(PowerPcAsm::addi(4, 4, 0x214));     // GameInfo->GameSelectInfo -> r4
    asm_.append(PowerPcAsm::lbz(4, 0x226, 4));      // GameInfo->GameSelectInfo->BasicRules -> r4

    QSet<qint8> mapSets;
    for (auto &mapDescriptor: mapDescriptors) {
        if (mapDescriptor.mapSet != -1) {
            mapSets.insert(mapDescriptor.mapSet);
        }
    }
    QList<qint8> mapSetsSorted = mapSets.values();
    std::sort(mapSetsSorted.begin(), mapSetsSorted.end());
    qint8 lastMapSet = mapSetsSorted.last();

    for (qint8 mapSet: qAsConst(mapSetsSorted)) {
        QSet<qint8> zones;
        for (auto &mapDescriptor: mapDescriptors) {
            if (mapDescriptor.mapSet == mapSet && mapDescriptor.zone != -1) {
                zones.insert(mapDescriptor.zone);
            }
        }
        QList<qint8> zonesSorted = zones.values();
        std::sort(zonesSorted.begin(), zonesSorted.end());

        asm_l2.clear();
        for (quint8 zone: qAsConst(zonesSorted)) {
            short count = std::count_if(mapDescriptors.begin(), mapDescriptors.end(), [&](const MapDescriptor &descriptor) {
                int d_zone = descriptor.zone;
                if (d_zone == 0) {
                    d_zone = 1;
                } else if (d_zone == 1) {
                    d_zone = 0;
                }
                int d_mapSet = descriptor.mapSet;
                if (d_mapSet == 0) {
                    d_mapSet = 1;
                } else if (d_mapSet == 1) {
                    d_mapSet = 0;
                }
                return zone == d_zone && mapSet == d_mapSet;
            });
            asm_l2.append(PowerPcAsm::cmpwi(3, zone));
            asm_l2.append(PowerPcAsm::bne(3));
            asm_l2.append(PowerPcAsm::li(3, count));
            asm_l2.append(PowerPcAsm::blr());
        }

        if(mapSet != lastMapSet) {
            asm_.append(PowerPcAsm::cmpwi(4, mapSet));
            asm_.append(PowerPcAsm::bne(asm_l2.size() + 1));
        }
        asm_.append(asm_l2);
    }

    return asm_;
}

QVector<quint32> MapSetZoneOrder::writeSubroutineGetMapsInZone(const AddressMapper &, const std::vector<MapDescriptor> &mapDescriptors, quint32 /*mapSetZoneOrderTable*/, quint32 entryAddr, quint32 returnAddr) {
    //PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(mapSetZoneOrderTable);

    // precondition:  r5  MapSet
    //               r29  _ZONE_TYPE
    //               r30  int* array containing map ids
    //                r3,r4,r6,r7,r31  unused
    // postcondition: r3  num maps (must be same as in SubroutineGetNumMapsInZone)
    QVector<quint32> asm_, asm_l2, asm_l3;

    asm_.append(PowerPcAsm::li(3, 0));
    QSet<qint8> mapSets;
    for (auto &mapDescriptor: mapDescriptors) {
        if (mapDescriptor.mapSet != -1) {
            mapSets.insert(mapDescriptor.mapSet);
        }
    }
    // not really needed to sort, but helps in debugging
    QList<qint8> mapSetsSorted = mapSets.values();
    std::sort(mapSetsSorted.begin(), mapSetsSorted.end());

    for (qint8 mapSet: qAsConst(mapSetsSorted)) {
        asm_.append(PowerPcAsm::cmpwi(5, mapSet));
        QSet<qint8> zones;
        for (auto &mapDescriptor: mapDescriptors) {
            if (mapDescriptor.mapSet == mapSet && mapDescriptor.zone != -1) {
                zones.insert(mapDescriptor.zone);
            }
        }
        // same again, helps in debugging
        QList<qint8> zonesSorted = zones.values();
        std::sort(zonesSorted.begin(), zonesSorted.end());
        asm_l2.clear();
        for (quint8 zone: qAsConst(zonesSorted)) {
            asm_l2.append(PowerPcAsm::cmpwi(29, zone));
            std::vector<MapDescriptor> maps;
            for (auto &mapDescriptor: mapDescriptors) {
                if (mapDescriptor.mapSet == mapSet) {
                    if (zone == 0) {
                        if (mapDescriptor.zone == 1) maps.push_back(mapDescriptor);
                    } else if (zone == 1) {
                        if (mapDescriptor.zone == 0) maps.push_back(mapDescriptor);
                    } else {
                        if (mapDescriptor.zone == zone) maps.push_back(mapDescriptor);
                    }
                }
            }
            std::stable_sort(maps.begin(), maps.end(), [&](const MapDescriptor &a, const MapDescriptor &b) { return a.order < b.order; });
            short i = 0;
            asm_l3.clear();
            asm_l3.append(PowerPcAsm::li(3, (short)maps.size()));
            for (auto &map: maps) {
                short mapId = std::find(mapDescriptors.begin(), mapDescriptors.end(), map) - mapDescriptors.begin();
                //var mapDescriptor = mapDescriptors[i];
                asm_l3.append(PowerPcAsm::li(4, mapId));
                asm_l3.append(PowerPcAsm::stw(4, i, 30));
                i += 4;
            }
            asm_l2.append(PowerPcAsm::bne(asm_l3.size() + 1));
            asm_l2.append(asm_l3);
        }
        asm_.append(PowerPcAsm::bne(asm_l2.size() + 1));
        asm_.append(asm_l2);
    }
    asm_.append(PowerPcAsm::b(entryAddr, asm_.size(), returnAddr));
    return asm_;
}

void MapSetZoneOrder::setVanillaMapSetZoneOrder(std::vector<MapDescriptor> &mapDescriptors) {
    for (int i=0; i<mapDescriptors.size(); ++i) {
        auto &mapDescriptor = mapDescriptors[i];
        mapDescriptor.mapSet = VanillaDatabase::getVanillaMapSet(i);
        mapDescriptor.zone = VanillaDatabase::getVanillaZone(i);
        if (mapDescriptor.zone == 0) {
            mapDescriptor.zone = 1;
        } else if (mapDescriptor.zone == 1) {
            mapDescriptor.zone = 0;
        }
        mapDescriptor.order = VanillaDatabase::getVanillaOrder(i);
    }
}
