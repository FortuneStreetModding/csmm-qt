#include "doliotable.h"

void DolIOTable::readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) {
    bool isVanilla = readIsVanilla(stream, addressMapper);
    quint32 addr = readTableAddr(stream, addressMapper, isVanilla);
    if (addr) {
        stream.device()->seek(addressMapper.toFileAddress(addr));
    }
    readAsm(stream, mapDescriptors, addressMapper, isVanilla);
}
