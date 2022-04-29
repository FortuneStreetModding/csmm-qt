#include "internalnametable.h"
#include "lib/powerpcasm.h"
#include <cstring>

quint32 InternalNameTable::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(allocate(descriptor.internalName));
    return allocate(table, "InternalNameTable");
}

void InternalNameTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    //qDebug() << addressMapper.boomToFileAddress(0x804363b8);
    stream.device()->seek(addressMapper.boomToFileAddress(0x804363b8)); stream.writeRawData("NAME", 4);
    // Store the pointer to the table to some unused address. The game does not use the internal name, but CSMM uses it for the name of the map descriptor file.
    stream.device()->seek(addressMapper.boomToFileAddress(0x804363bc)); stream << PowerPcAsm::lis(5, v.upper) << PowerPcAsm::addi(5, 5, v.lower);
}

void InternalNameTable::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x8);
    }
    for (auto &mapDescriptor: mapDescriptors) {
        quint32 addr;
        stream >> addr;
        mapDescriptor.internalName = resolveAddressToString(addr, stream, addressMapper).trimmed();
        // clear the internal name of characters which are not allowed in a file system
        mapDescriptor.internalName.replace(QRegularExpression("[<>:/\\|?*\"]+"), "");
        if (isVanilla) {
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x38 - 0x04);
        }
    }
}

quint32 InternalNameTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) {
    stream.device()->seek(addressMapper.boomToFileAddress(isVanilla ? 0x801cca70 : 0x804363bc));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 InternalNameTable::readTableRowCount(QDataStream &, const AddressMapper &, bool) {
    return -1;
}

bool InternalNameTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    //qDebug() << addressMapper.boomToFileAddress(0x804363b8);
    stream.device()->seek(addressMapper.boomToFileAddress(0x804363b8));
    char data[4];
    stream.readRawData(data, 4);
    //qDebug() << QByteArray(data, 4);
    return std::memcmp(data, "NAME", 4) != 0;
}
