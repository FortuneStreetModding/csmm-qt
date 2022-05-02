#include "mapdescriptiontable.h"
#include "lib/powerpcasm.h"

quint32 MapDescriptionTable::writeTable(const QVector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(descriptor.descMsgId);
    return allocate(table, "MapDescriptionTable");
}

void MapDescriptionTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    short tableRowCount = (short)mapDescriptors.size();
    quint32 mapDescriptionTableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(mapDescriptionTableAddr);
    // HACK: Expand the description message ID table
    // subi r3,r3,0x15                                     -> nop
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021214c)); stream << PowerPcAsm::nop();
    // cmpwi r3,0x12                                       -> cmpwi r3,tableRowCount
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212158)); stream << PowerPcAsm::cmpwi(3, tableRowCount);
    // r4 <- 0x80436bc0                                    -> r4 <- mapDescriptionTableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212164)); stream << PowerPcAsm::lis(4, v.upper);
    stream.skipRawData(4); stream << PowerPcAsm::addi(4, 4, v.lower);
}

void MapDescriptionTable::readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        readVanillaTable(stream, mapDescriptors);
    } else {
        for (auto &mapDescriptor: mapDescriptors) {
            stream >> mapDescriptor.descMsgId;
        }
    }
}

quint32 MapDescriptionTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212164));
    quint32 lisOpcode, dummy, addiOpcode;
    stream >> lisOpcode >> dummy >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 MapDescriptionTable::readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x80212158));
    quint32 opcode; stream >> opcode;
    return PowerPcAsm::getOpcodeParameter(opcode);
}

bool MapDescriptionTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x8021214c));
    quint32 opcode; stream >> opcode;
    // subi r3,r3,0x15
    return opcode == PowerPcAsm::subi(3, 3, 0x15);
}

void MapDescriptionTable::readVanillaTable(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors) {
    quint32 descMsgIdTable[18];
    for (auto &elem: descMsgIdTable) {
        stream >> elem;
    }
    int j = 0;
    for (auto &mapDescriptor: mapDescriptors) {
        if (mapDescriptor.unlockKey < 18) {
            mapDescriptor.descMsgId = descMsgIdTable[j];
            ++j;
            if (j == 18) {
                j = 0;
            }
        }
    }
}

