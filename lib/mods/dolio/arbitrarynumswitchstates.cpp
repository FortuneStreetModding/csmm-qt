#include "arbitrarynumswitchstates.h"
#include "lib/powerpcasm.h"

void ArbitraryNumSwitchStates::readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors)
{
    // crab nothing to do crab
}

void ArbitraryNumSwitchStates::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors)
{
    // Allow switch button district/destination ID to handle # of states other than 2 or 4

    // StopSwitch
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007f120));

    // cmpwi r4, 4 -> cmpwi r4, 0
    stream << PowerPcAsm::cmpwi(4, 0);
    stream.skipRawData(4);
    // li r5, 2 -> mr r5, r4
    stream << PowerPcAsm::mr(5, 4);
    stream.skipRawData(4);
    // li r5, 4 -> li r5, 2
    stream << PowerPcAsm::li(5, 2);

    // StopPlace
    stream.device()->seek(addressMapper.boomToFileAddress(0x8007f5a0));
    // li r5, 2 -> mr r5, r0
    stream << PowerPcAsm::mr(5, 0);
    stream.skipRawData(4);
    // cmpwi r0, 4 -> cmpwi r0, 0
    stream << PowerPcAsm::cmpwi(0, 0);
    stream.skipRawData(8);
    // mr r5, r7 -> li r5, 2
    stream << PowerPcAsm::li(5, 2);

    // Allow >4 states
    // TODO add implementation of this
}
