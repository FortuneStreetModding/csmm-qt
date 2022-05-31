#include "arbitrarynumswitchstates.h"
#include "lib/powerpcasm.h"

void ArbitraryNumSwitchStates::readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors)
{
    // crab nothing to do crab
}

void ArbitraryNumSwitchStates::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors)
{
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007f120));

    // TODO potentially replace nops with branch and free space

    // cmpwi r4, 4 -> mr r5, r4
    stream << PowerPcAsm::mr(5, 4);
    // addi r6, r31, 1 -> nop
    stream << PowerPcAsm::nop();
    // li r5, 2 -> nop
    stream << PowerPcAsm::nop();
    // bne LAB_8007f134 -> nop
    stream << PowerPcAsm::nop();
    // li r5, 4 -> nop
    stream << PowerPcAsm::nop();
}
