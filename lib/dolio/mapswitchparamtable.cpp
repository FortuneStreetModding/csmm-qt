#include "mapswitchparamtable.h"
#include "lib/powerpcasm.h"

quint32 MapSwitchParamTable::writeTable(const QVector<MapDescriptor> &descriptors) {
    QVector<quint32> mapSwitchParamTable;
    for (auto &descriptor: descriptors) {
        if (descriptor.switchRotationOrigins.size() == 0) {
            mapSwitchParamTable.append(0);
        } else {
            QByteArray arr;
            QDataStream arrStream(&arr, QIODevice::WriteOnly);

            // write 4-byte floats
            arrStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

            arrStream << (quint32)descriptor.switchRotationOrigins.size();
            for (auto &originPoint: descriptor.switchRotationOrigins) {
                arrStream << originPoint.x << (quint32)0 << originPoint.y;
            }
            quint32 loopingModeConfigAddr = allocate(arr, "MapRotationOriginPoints for " + descriptor.internalName);
            mapSwitchParamTable.append(loopingModeConfigAddr);
        }
    }
    return allocate(mapSwitchParamTable, "MapSwitchParamTable");
}

void MapSwitchParamTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // --- Update Table Addr ---
    // mulli r0,r3,0x38 ->  mulli r0,r3,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb28)); stream << PowerPcAsm::mulli(0, 3, 0x04);
    // r3 <- 0x80428e50 ->  r3 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb2c)); stream << PowerPcAsm::lis(3, v.upper) << PowerPcAsm::addi(3, 3, v.lower);
    // lwz r3,0x4(r3)   ->  lwz r3,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb38)); stream << PowerPcAsm::lwz(3, 0x0, 3);
}

void MapSwitchParamTable::readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x28);
    }
    for (auto &mapDescriptor: mapDescriptors) {
        quint32 addr;
        stream >> addr;
        qint64 pos = stream.device()->pos();
        readRotationOriginPoints(addr, stream, mapDescriptor, addressMapper);
        stream.device()->seek(pos);
        if (isVanilla) {
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x38 - 0x04);
        }
    }
}

quint32 MapSwitchParamTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb2c));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 MapSwitchParamTable::readTableRowCount(QDataStream &, const AddressMapper &, bool) {
    return -1;
}

bool MapSwitchParamTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb28));
    quint32 opcode; stream >> opcode;
    // mulli r0,r3,0x38
    return opcode == PowerPcAsm::mulli(0, 3, 0x38);
}

void MapSwitchParamTable::readRotationOriginPoints(quint32 address, QDataStream &stream, MapDescriptor &mapDescriptor, const AddressMapper &addressMapper) {
    mapDescriptor.switchRotationOrigins.clear();
    // Special case handling: in the original game these values are initialized at run time only. So we need to hardcode them:
    if (address == addressMapper.boomStreetToStandard(0x806b8df0)) { // magmageddon
        // no points
    } else if (address == addressMapper.boomStreetToStandard(0x8047d598)) { // collosus
        mapDescriptor.switchRotationOrigins = {{-288, -32}, {288, -32}};
    } else if (address == addressMapper.boomStreetToStandard(0x8047d5b4)) { // observatory
        mapDescriptor.switchRotationOrigins = {{0, 0}};
    } else if (addressMapper.canConvertToFileAddress(address)) {
        stream.device()->seek(addressMapper.toFileAddress(address));
        quint32 originPointCount; stream >> originPointCount;
        for (quint32 i = 0; i < originPointCount; i++) {
            OriginPoint point;
            stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
            float dummy;
            stream >> point.x >> dummy /* ignore Z value */ >> point.y;
            mapDescriptor.switchRotationOrigins.append(point);
        }
    }
}
