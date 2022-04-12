#include "allocatedescriptorcount.h"

#include "lib/powerpcasm.h"

void AllocateDescriptorCount::readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors)
{
    stream.device()->seek(addressMapper.boomToFileAddress(0x801cca30));
    quint32 opcode;
    stream >> opcode;
    qint16 count = PowerPcAsm::getOpcodeParameter(opcode);
    mapDescriptors.resize(count);
}

void AllocateDescriptorCount::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors)
{
    // crab nothing to do crab
}