#include "changemusiconswitch.h"
#include "lib/powerpcasm.h"

void ChangeMusicOnSwitch::readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors)
{
}

void ChangeMusicOnSwitch::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors)
{
    auto oldSwitchStateAddr = allocate(QByteArray(4, '\0'), "Old Switch State", false);

    auto subroutineAddr = allocate(lookAtSwitchStateSubroutine(addressMapper, 0, oldSwitchStateAddr), "Look At Switch State Subroutine", false);
    auto subroutine = lookAtSwitchStateSubroutine(addressMapper, subroutineAddr, oldSwitchStateAddr);
    stream.device()->seek(addressMapper.toFileAddress(subroutineAddr));
    for (auto word: subroutine) stream << word;

    // originally bge 0x800d31d0
    auto hijackAddr = addressMapper.boomStreetToStandard(0x800d31b8);
    stream.device()->seek(addressMapper.toFileAddress(hijackAddr));
    stream << PowerPcAsm::b(hijackAddr, subroutineAddr);
}

QVector<quint32> ChangeMusicOnSwitch::lookAtSwitchStateSubroutine(const AddressMapper &addressMapper, quint32 startAddr, quint32 oldSwitchStateAddr) {
    using namespace PowerPcAsm;

    auto switchStatePair = make16bitValuePair(addressMapper.boomStreetToStandard(0x80552410));
    auto oldSwitchStatePair = make16bitValuePair(oldSwitchStateAddr);

    QVector<quint32> asm_;

    LabelTable labels;

    // r7 <- &switchState
    asm_.push_back(lis(7, switchStatePair.upper));
    asm_.push_back(addi(7, 7, switchStatePair.lower));
    // r5 <- *r7
    asm_.push_back(lwz(5, 0x0, 7));
    // r7 <- &oldSwitchState
    asm_.push_back(lis(7, oldSwitchStatePair.upper));
    asm_.push_back(addi(7, 7, oldSwitchStatePair.lower));
    // r4 <- *r7
    asm_.push_back(lwz(4, 0x0, 7));

    // proceed to 0x800d31bc if old comparison <
    asm_.push_back(blt(labels, "proceed", asm_));
    // proceed to 0x800d31bc if r5 != r4
    asm_.push_back(cmplw(5, 4));
    asm_.push_back(bne(labels, "proceed", asm_));

    // otherwise jump to 0x800d31d0
    asm_.push_back(b(startAddr, asm_.size(), addressMapper.boomStreetToStandard(0x800d31d0)));

    labels.define("proceed", asm_);
    // update oldSwitchState
    asm_.push_back(stw(5, 0x0, 7));
    asm_.push_back(b(startAddr, asm_.size(), addressMapper.boomStreetToStandard(0x800d31bc)));
#if 0
    labels.define("proceed2", asm_);
    // update oldSwitchState
    asm_.push_back(stw(3, 0x0, 5));
    asm_.push_back(b(startAddr, asm_.size(), addressMapper.boomStreetToStandard(0x800d31e0)));
#endif
    return asm_;
}
