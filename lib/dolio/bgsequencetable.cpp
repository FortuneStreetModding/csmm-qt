#include "bgsequencetable.h"
#include "lib/powerpcasm.h"

quint32 BGSequenceTable::writeTable(const QVector<MapDescriptor> &descriptors, quint32 bgSequenceMarioStadium) {
    QVector<quint32> table;
    for (auto &descriptor: descriptors) table.append(descriptor.background == "bg004" ? bgSequenceMarioStadium : 0);
    return allocate(table);
}

void BGSequenceTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    // hardcoded virtual address for the parameter table on how the Miis are being animated to play baseball in the background
    quint32 bgSequenceMarioStadium = addressMapper.boomStreetToStandard(0x80428968);
    quint32 tableAddr = writeTable(mapDescriptors, bgSequenceMarioStadium);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    // --- Update Table Addr ---
    // mulli r0,r3,0x38 ->  mulli r0,r3,0x04
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb70)); stream << PowerPcAsm::mulli(0, 3, 0x04);
    // r3 <- 0x80428e50 ->  r3 <- tableAddr
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb74)); stream << PowerPcAsm::lis(3, v.upper) << PowerPcAsm::addi(3, 3, v.lower);
    // lwz r3,0x34(r3)   ->  lwz r3,0x0(r3)
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb80)); stream << PowerPcAsm::lwz(3, 0x0, 3);
}

void BGSequenceTable::readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (isVanilla) {
        stream.skipRawData(0x4);
    }
    for (int i=0; i<mapDescriptors.size(); ++i) {
        stream.skipRawData(4);
        if (isVanilla) {
            // in vanilla main.dol the table has other stuff in it like bgm id, map frb files, etc.
            // this we need to skip to go the next target amount in the table
            stream.skipRawData(0x38 - 0x04);
        }
    }
}

quint32 BGSequenceTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb74));
    quint32 lisOpcode, addiOpcode;
    stream >> lisOpcode >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

qint16 BGSequenceTable::readTableRowCount(QDataStream &, const AddressMapper &, bool) {
    return -1;
}

bool BGSequenceTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x801ccb70));
    quint32 opcode; stream >> opcode;
    // mulli r0,r3,0x38
    return opcode == PowerPcAsm::mulli(0, 3, 0x38);
}
