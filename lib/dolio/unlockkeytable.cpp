#include "unlockkeytable.h"
#include "lib/powerpcasm.h"
#include "lib/vanilladatabase.h"

quint32 UnlockKeyTable::writeTable(const QVector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    // TODO
    return allocate(table, "UnlockKeysTable");
}

void UnlockKeyTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // TODO
}

void UnlockKeyTable::readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    for(int i=0;i<mapDescriptors.length();i++) {
        auto &mapDescriptor = mapDescriptors[i];
        if (isVanilla) {
            mapDescriptor.unlockKeys = VanillaDatabase::getVanillaUnlockKeys(i);
        } else {
            // TODO
        }
    }
}

quint32 UnlockKeyTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    // TODO
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb5c));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 UnlockKeyTable::readTableRowCount(QDataStream &, const AddressMapper &, bool) {
    return -1;
}

bool UnlockKeyTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    // TODO
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb58));
    quint32 opcode; stream >> opcode;
    // mulli r0,r3,0x38
    return opcode == PowerPcAsm::mulli(0, 3, 0x38);
}
