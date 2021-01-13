#include "mapsetzoneorder.h"
#include "lib/powerpcasm.h"
#include "lib/vanilladatabase.h"

quint32 MapSetZoneOrder::writeTable(const QVector<MapDescriptor> &descriptors) {
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
    return allocate(mapSetZoneOrderTable);
}

void MapSetZoneOrder::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);

    // --- Game::GameSequenceDataAdapter::GetNumMapsInZone ---
    quint32 subroutineGetNumMapsInZone = allocate(writeSubroutineGetNumMapsInZone(mapDescriptors));
    quint8 hijackAddr = addressMapper.boomStreetToStandard(0x8020f3ac);
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    // li r3,0x6 ->  b subroutineGetNumMapsInZone
    stream << PowerPcAsm::b(hijackAddr, subroutineGetNumMapsInZone);

    // --- Game::GameSequenceDataAdapter::GetMapsInZone ---
    hijackAddr = addressMapper.boomStreetToStandard(0x8020f454);
    quint32 returnAddr = addressMapper.boomStreetToStandard(0x8020f554);
    quint32 subroutineGetMapsInZone = allocate(writeSubroutineGetMapsInZone(addressMapper, mapDescriptors, tableAddr, 0, returnAddr));
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

void MapSetZoneOrder::readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
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

QVector<quint32> MapSetZoneOrder::writeSubroutineGetNumMapsInZone(const QVector<MapDescriptor> &mapDescriptors) {
    // precondition:  r3  _ZONE_TYPE
    // postcondition: r3  num maps
    QVector<quint32> asm_;
    for (short i = 0; i < 6; i++) {
        asm_.append(PowerPcAsm::cmpwi(3, i));
        asm_.append(PowerPcAsm::bne(3));
        short count = std::count_if(mapDescriptors.begin(), mapDescriptors.end(), [&](const MapDescriptor &descriptor) {
            return descriptor.zone == i && descriptor.mapSet == 0;
        });
        asm_.append(PowerPcAsm::li(3, count));
        asm_.append(PowerPcAsm::blr());
    }
    asm_.append(PowerPcAsm::blr());
    return asm_;
}

QVector<quint32> MapSetZoneOrder::writeSubroutineGetMapsInZone(const AddressMapper &, const QVector<MapDescriptor> &mapDescriptors, quint32 /*mapSetZoneOrderTable*/, quint32 entryAddr, quint32 returnAddr) {
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
    for (qint8 mapSet: mapSets) {
        asm_.append(PowerPcAsm::cmpwi(5, mapSet));
        QSet<qint8> zones;
        for (auto &mapDescriptor: mapDescriptors) {
            if (mapDescriptor.mapSet == mapSet && mapDescriptor.zone != -1) {
                zones.insert(mapDescriptor.zone);
            }
        }
        asm_l2.clear();
        for (quint8 zone: zones) {
            asm_l2.append(PowerPcAsm::cmpwi(29, zone));
            QVector<MapDescriptor> maps;
            for (auto &mapDescriptor: mapDescriptors) {
                if (mapDescriptor.mapSet == mapSet) {
                    if (zone == 0) {
                        if (mapDescriptor.zone == 1) maps.append(mapDescriptor);
                    } else if (zone == 1) {
                        if (mapDescriptor.zone == 0) maps.append(mapDescriptor);
                    } else {
                        if (mapDescriptor.zone == zone) maps.append(mapDescriptor);
                    }
                }
            }
            std::stable_sort(maps.begin(), maps.end(), [&](const MapDescriptor &a, const MapDescriptor &b) { return a.order < b.order; });
            short i = 0;
            asm_l3.clear();
            asm_l3.append(PowerPcAsm::li(3, (short)maps.size()));
            for (auto &map: maps) {
                short mapId = (short)mapDescriptors.indexOf(map);
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

void MapSetZoneOrder::setVanillaMapSetZoneOrder(QVector<MapDescriptor> &mapDescriptors) {
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
