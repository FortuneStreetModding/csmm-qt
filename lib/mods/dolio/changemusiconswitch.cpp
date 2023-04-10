#include "changemusiconswitch.h"
#include "lib/powerpcasm.h"

void ChangeMusicOnSwitch::readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors)
{
}

void ChangeMusicOnSwitch::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors)
{
#if 0
    auto addr = allocate(changeBgmSubroutine(addressMapper, 0), "Change Music On Switch Subroutine", false);
    auto subroutine = changeBgmSubroutine(addressMapper, addr);
    stream.device()->seek(addressMapper.toFileAddress(addr));
    for (auto word: subroutine) stream << word;

    auto hijackAddr = addressMapper.boomStreetToStandard(0x800fabac);
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    stream << PowerPcAsm::b(hijackAddr, addr);

    // 0x800d3198
#endif
    auto oldSwitchStateAddr = allocate(QByteArray(1, '\0'), "Old Switch State", false);

    auto subroutineAddr = allocate(lookAtSwitchStateSubroutine(addressMapper, 0, oldSwitchStateAddr), "Look At Switch State Subroutine", false);
    auto subroutine = lookAtSwitchStateSubroutine(addressMapper, subroutineAddr, oldSwitchStateAddr);
    stream.device()->seek(addressMapper.toFileAddress(subroutineAddr));
    for (auto word: subroutine) stream << word;

    auto hijackAddr = addressMapper.boomStreetToStandard(0x800d31b8);
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    stream << PowerPcAsm::b(hijackAddr, subroutineAddr);
}

QVector<quint32> ChangeMusicOnSwitch::lookAtSwitchStateSubroutine(const AddressMapper &addressMapper, quint32 startAddr, quint32 oldSwitchStateAddr) {
    using namespace PowerPcAsm;

    auto switchStatePair = make16bitValuePair(addressMapper.boomStreetToStandard(0x80552410));
    auto oldSwitchStatePair = make16bitValuePair(addressMapper.boomStreetToStandard(oldSwitchStateAddr));

    QVector<quint32> asm_;

    LabelTable labels;

    asm_.push_back(lis(5, switchStatePair.upper));
    asm_.push_back(addi(5, 5, switchStatePair.lower));
    asm_.push_back(lwz(3, 0x0, 5));
    asm_.push_back(lis(5, oldSwitchStatePair.upper));
    asm_.push_back(addi(5, 5, oldSwitchStatePair.lower));
    asm_.push_back(lwz(4, 0x0, 5));

    asm_.push_back(blt(labels, "proceed", asm_));
    asm_.push_back(cmplw(3, 4));
    asm_.push_back(bne(labels, "proceed", asm_));
    asm_.push_back(b(startAddr, asm_.size(), addressMapper.boomStreetToStandard(0x800d31d0)));
    labels.define("proceed", asm_);
    asm_.push_back(stw(3, 0x0, 5));
    asm_.push_back(b(startAddr, asm_.size(), addressMapper.boomStreetToStandard(0x800d31bc)));

    return asm_;
}


#if 0
QVector<quint32> ChangeMusicOnSwitch::changeBgmSubroutine(const AddressMapper &addressMapper, quint32 startAddr)
{
    auto gameManagerPair = PowerPcAsm::make16bitValuePair(addressMapper.boomStreetToStandard(0x8081794c));

    QVector<quint32> asm_;
    asm_.push_back(PowerPcAsm::lis(4, gameManagerPair.upper));
    asm_.push_back(PowerPcAsm::addi(4, 4, gameManagerPair.lower));
    asm_.push_back(PowerPcAsm::lwz(3, 0x0, 4)); // r3 <- gameManager
    asm_.push_back(PowerPcAsm::li(4, MusicType::auction)); // r4
    asm_.push_back(PowerPcAsm::li(5, 0)); // r5
    asm_.push_back(PowerPcAsm::bl(startAddr, asm_.size(), addressMapper.boomStreetToStandard(0x800d3150))); // ChangeBGM
    asm_.push_back(PowerPcAsm::b(startAddr, asm_.size(), addressMapper.boomStreetToStandard(0x800fac38)));

    return asm_;
}

QVector<quint32> ChangeMusicOnSwitch::compareParam1Subroutine(const AddressMapper &addressMapper, quint32 startAddr)
{
    QVector<quint32> asm_;

    asm_.push_back(PowerPcAsm::b(startAddr, asm_.size(), addressMapper.boomStreetToStandard(0x800d319c)));
    asm_.push_back(PowerPcAsm::b(startAddr, asm_.size(), addressMapper.boomStreetToStandard(0x800d320c)));

    return asm_;
}

#endif
