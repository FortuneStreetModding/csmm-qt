#include "internalnametable.h"
#include "lib/powerpcasm.h"
#include <cstring>

void InternalNameTable::loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) {
    QFile addrFile(QDir(root).filePath(ADDRESS_FILE.data()));
    if (addrFile.open(QFile::ReadOnly)) {
        QDataStream addrStream(&addrFile);
        addrStream >> internalNameDataAddr;
    }
    DolIOTable::loadFiles(root, gameInstance, modList);
}

void InternalNameTable::saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) {
    DolIOTable::saveFiles(root, gameInstance, modList);
    DolIO::saveFiles(root, gameInstance, modList);
    QSaveFile addrFile(QDir(root).filePath(ADDRESS_FILE.data()));
    if (addrFile.open(QFile::WriteOnly)) {
        QDataStream addrStream(&addrFile);
        addrStream << internalNameDataAddr;
        addrFile.commit();
    }
}

quint32 InternalNameTable::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(allocate(descriptor.internalName));
    return allocate(table, "InternalNameTable");
}

void InternalNameTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    QByteArray internalNameInfo;
    QDataStream infoStream(&internalNameInfo, QFile::WriteOnly);

    // Store the pointer to the table to some unused address. The game does not use the internal name, but CSMM uses it for the name of the map descriptor file.
    infoStream << PowerPcAsm::lis(5, v.upper) << PowerPcAsm::addi(5, 5, v.lower);

    internalNameDataAddr = allocate(internalNameInfo, "Internal Name Pointer");
}

static const QRegularExpression CLEAN_INTERNAL_NAME("[<>:/\\|?*\"]+");

void InternalNameTable::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x8);
    }
    for (auto &mapDescriptor: mapDescriptors) {
        quint32 addr;
        stream >> addr;
        mapDescriptor.internalName = resolveAddressToString(addr, stream, addressMapper).trimmed();
        // clear the internal name of characters which are not allowed in a file system
        mapDescriptor.internalName.replace(CLEAN_INTERNAL_NAME, "");
        if (isVanilla) {
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x38 - 0x04);
        }
    }
}

quint32 InternalNameTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) {
    stream.device()->seek(addressMapper.boomToFileAddress(isVanilla ? 0x801cca70 : internalNameDataAddr));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

bool InternalNameTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    return internalNameDataAddr == 0;
}
