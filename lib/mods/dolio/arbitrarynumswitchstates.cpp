#include "arbitrarynumswitchstates.h"
#include "lib/powerpcasm.h"

void ArbitraryNumSwitchStates::readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors)
{
    // crab nothing to do crab
}

void ArbitraryNumSwitchStates::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors)
{
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007f120));

    // cmpwi r4, 4 -> cmpwi r4, 0
    stream << PowerPcAsm::cmpwi(4, 0);
    stream.skipRawData(4);
    // li r5, 2 -> mr r5, r4
    stream << PowerPcAsm::mr(5, 4);
    stream.skipRawData(4);
    //li r5, 4 -> li r5, 2
    stream << PowerPcAsm::li(5, 2);
}
