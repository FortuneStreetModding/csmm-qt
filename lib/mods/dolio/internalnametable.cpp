#include "internalnametable.h"
#include "lib/powerpcasm.h"
#include <cstring>

static const QRegularExpression CLEAN_INTERNAL_NAME("[<>:/\\|?*\"]+");

void InternalNameTable::loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) {
    QFile nameListFile(QDir(root).filePath(NAME_LIST.data()));
    if (nameListFile.exists() && nameListFile.open(QFile::ReadOnly)) {
        QTextStream listStream(&nameListFile);
        listStream.setCodec("UTF-8");
        for (auto &descriptor: gameInstance->mapDescriptors()) {
            descriptor.internalName = listStream.readLine();
            // clear the internal name of characters which are not allowed in a file system
            descriptor.internalName.replace(CLEAN_INTERNAL_NAME, "");
        }
    } else {
        QFile addrFile(QDir(root).filePath(ADDRESS_FILE.data()));
        if (addrFile.open(QFile::ReadOnly)) {
            QDataStream addrStream(&addrFile);
            addrStream >> internalNameDataAddr;
        }
        DolIOTable::loadFiles(root, gameInstance, modList);
    }
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
    QSaveFile nameListFile(QDir(root).filePath(NAME_LIST.data()));
    if (nameListFile.open(QFile::WriteOnly)) {
        QTextStream listStream(&nameListFile);
        listStream.setCodec("UTF-8");
        for (auto &descriptor: gameInstance->mapDescriptors()) {
            listStream << descriptor.internalName << Qt::endl;
        }
        nameListFile.commit();
    }
}

quint32 InternalNameTable::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(allocate(descriptor.internalName));
    return allocate(table, "InternalNameTable");
}

void InternalNameTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    internalNameDataAddr = 0;
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
