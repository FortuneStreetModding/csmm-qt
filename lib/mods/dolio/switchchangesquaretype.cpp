#include "switchchangesquaretype.h"

void SwitchChangeSquareType::readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) {
    // crab nothing to do crab
}

void SwitchChangeSquareType::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) {
    stream.device()->seek(0x00000000);
}
