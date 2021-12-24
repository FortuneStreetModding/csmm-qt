#include "nameddistricts.h"
#include "lib/powerpcasm.h"

void NamedDistricts::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) {
    quint32 tableAddr = writeTable(mapDescriptors);
    PowerPcAsm::Pair16Bit v = PowerPcAsm::make16bitValuePair(tableAddr);

    stream.device()->seek(addressMapper.boomToFileAddress(0x800f8458));
    // cmplwi r4,0x15    ->    cmplwi r4,len(descriptors)
    stream << PowerPcAsm::cmplwi(4, mapDescriptors.size());

    stream.device()->seek(addressMapper.boomToFileAddress(0x800f8484));
    // r29 <- 0x80417460    ->    r29 <- tableAddr
    stream << PowerPcAsm::lis(29, v.upper);
    stream.skipRawData(4);
    stream << PowerPcAsm::addi(29, 29, v.lower);
}

quint32 NamedDistricts::writeTable(const QVector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (auto &desc: descriptors) {
        table.append(desc.districtNameIds[0]);
        table.append(desc.districtNameIds.size());
    }
    return allocate(table, "district localization table");
}

bool NamedDistricts::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x800f8458));
    quint32 opcode; stream >> opcode;
    return opcode == PowerPcAsm::cmplwi(4, 0x15);
}

qint16 NamedDistricts::readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x800f8458));
    quint32 opcode; stream >> opcode;
    return PowerPcAsm::getOpcodeParameter(opcode);
}

quint32 NamedDistricts::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool)  {
    stream.device()->seek(addressMapper.boomToFileAddress(0x800f8484));
    quint32 lisOpcode, dummy, addiOpcode;
    stream >> lisOpcode >> dummy >> addiOpcode;
    return PowerPcAsm::make32bitValueFromPair(lisOpcode, addiOpcode);
}

void NamedDistricts::readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (!isVanilla) {
        for (auto &mapDescriptor: mapDescriptors) {
            quint32 startVal, tableLen;
            stream >> startVal >> tableLen;
            //qDebug() << startVal << tableLen;
            mapDescriptor.districtNameIds.clear();
            for (quint32 i=startVal; i<startVal+tableLen; ++i) {
                mapDescriptor.districtNameIds.append(i);
            }
        }
    } else {
        for (int i=0; i<0x15; ++i) {
            quint32 startVal, tableLen;
            stream >> startVal >> tableLen;
            // loop map descriptors by mod 21
            for (int j = i; j < mapDescriptors.size(); j += 0x15) {
                mapDescriptors[j].districtNameIds.clear();
                for (quint32 k=startVal; k<startVal+tableLen; ++k) {
                    mapDescriptors[j].districtNameIds.append(k);
                }
            }
        }
    }
}

