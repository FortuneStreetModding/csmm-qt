#include "forcesimulatedbuttonpress.h"
#include "lib/powerpcasm.h"

void ForceSimulatedButtonPress::loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) {
    DolIO::loadFiles(root, gameInstance, modList);
    QFile addrFile(QDir(root).filePath(ADDRESS_FILE.data()));
    if (addrFile.open(QFile::ReadOnly)) {
        QDataStream addrStream(&addrFile);
        addrStream >> forceSimulatedButtonPressAddr;
    }
}

void ForceSimulatedButtonPress::saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) {
    DolIO::saveFiles(root, gameInstance, modList);
    QSaveFile addrFile(QDir(root).filePath(ADDRESS_FILE.data()));
    if (addrFile.open(QFile::WriteOnly)) {
        QDataStream addrStream(&addrFile);
        addrStream << forceSimulatedButtonPressAddr;
        addrFile.commit();
    }
}

void ForceSimulatedButtonPress::readAsm(QDataStream &, const AddressMapper &, std::vector<MapDescriptor> &) {}

void ForceSimulatedButtonPress::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &) {
    quint32 hijackAddr = addressMapper.boomStreetToStandard(0x802bb120);
    quint32 returnAddr = addressMapper.boomStreetToStandard(0x802bb124);

    forceSimulatedButtonPressAddr = allocate(QByteArray(4, '\0'), "ForceSimulatedButtonPress", false);

    quint32 uploadSimulatedButtonPress = allocate(writeUploadSimulatedButtonPress(addressMapper, 0, returnAddr), "UploadSimulatedButtonPress");
    stream.device()->seek(addressMapper.toFileAddress(uploadSimulatedButtonPress));
    auto insts = writeUploadSimulatedButtonPress(addressMapper, uploadSimulatedButtonPress, returnAddr); // re-write the routine again since now we know where it is located in the main dol
    for (quint32 inst: qAsConst(insts)) stream << inst;
    // lwz r0,0x4(r3)                                                             -> b uploadSimulatedButtonPress
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr)); stream << PowerPcAsm::b(hijackAddr, uploadSimulatedButtonPress);
}
QVector<quint32> ForceSimulatedButtonPress::writeUploadSimulatedButtonPress(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 returnAddr) {
    auto forceSimulatedButtonPress = PowerPcAsm::make16bitValuePair(forceSimulatedButtonPressAddr);
    auto pressedButtonsBitArray = PowerPcAsm::make16bitValuePair(addressMapper.boomStreetToStandard(0x8078C880));

    return {
        PowerPcAsm::lis(6, forceSimulatedButtonPress.upper),                // |
        PowerPcAsm::addi(6, 6, forceSimulatedButtonPress.lower),            // | r6 <- &forceSimulatedButtonPress
        PowerPcAsm::lis(7, pressedButtonsBitArray.upper),                   // |
        PowerPcAsm::addi(7, 7, pressedButtonsBitArray.lower),               // | r7 <- &pressedButtonsBitArray
        PowerPcAsm::lwz(0, 0x0, 6),                                         // r0 <- forceSimulatedButtonPress
        PowerPcAsm::cmpwi(0, 0x0),                                          // if (forceSimulatedButtonPress != 0)
        PowerPcAsm::beq(4),                                                 // {
        PowerPcAsm::stw(0, 0x0, 7),                                         //   pressedButtonsBitArray <- forceSimulatedButtonPress
        PowerPcAsm::li(0, 0x0),                                             //   |
        PowerPcAsm::stw(0, 0x0, 6),                                         //   | forceSimulatedButtonPress <- 0
                                                                            // }
        PowerPcAsm::lwz(0, 0x4, 3),                                         // *replaced opcode*
        PowerPcAsm::b(routineStartAddress, 11 /*asm.Count*/, returnAddr)    // return
    };
}
