#include "mapgalaxyparamtable.h"
#include "lib/powerpcasm.h"

quint32 MapGalaxyParamTable::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) {
        if (descriptor.loopingMode == None) {
            table.append(0);
        } else {
            QByteArray data;
            QDataStream dataStream(&data, QIODevice::WriteOnly);
            // write 4-byte floats
            dataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
            dataStream << descriptor.loopingModeRadius << descriptor.loopingModeHorizontalPadding << descriptor.loopingModeVerticalSquareCount;
            table.append(allocate(data, "LoopingModeConfig for " + descriptor.internalName));
        }
    }
    return allocate(table, "MapGalaxyParamTable");
}

void MapGalaxyParamTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // --- Update Table Addr ---
    // mulli r0,r3,0x38 ->  mulli r0,r3,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb40)); stream << PowerPcAsm::mulli(0, 3, 0x04);
    // r3 <- 0x80428e50 ->  r3 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb44)); stream << PowerPcAsm::lis(3, v.upper) << PowerPcAsm::addi(3, 3, v.lower);
    // lwz r3,0x4(r3)   ->  lwz r3,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb50)); stream << PowerPcAsm::lwz(3, 0x0, 3);
}

void MapGalaxyParamTable::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x2C);
    }
    for (auto &mapDescriptor: mapDescriptors) {
        quint32 addr;
        stream >> addr;
        qint64 pos = stream.device()->pos();
        readLoopingModeConfig(addr, stream, mapDescriptor, addressMapper);
        stream.device()->seek(pos);
        if (isVanilla) {
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x38 - 0x04);
        }
    }
}

quint32 MapGalaxyParamTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb44));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

bool MapGalaxyParamTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb40));
    quint32 opcode; stream >> opcode;
    // mulli r0,r3,0x38
    return opcode == PowerPcAsm::mulli(0, 3, 0x38);
}

void MapGalaxyParamTable::readLoopingModeConfig(quint32 address, QDataStream &stream, MapDescriptor &mapDescriptor, const AddressMapper &addressMapper) {
    if (addressMapper.canConvertToFileAddress(address)) {
        stream.device()->seek(addressMapper.toFileAddress(address));
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        stream >> mapDescriptor.loopingModeRadius >> mapDescriptor.loopingModeHorizontalPadding >> mapDescriptor.loopingModeVerticalSquareCount;
    }
}
