#include "mutatortable.h"
#include "lib/powerpcasm.h"

quint32 MutatorTable::writeMutatorData(const MapDescriptor &descriptor) {
    QVector<quint32> data;
    for(auto& mutatorEnt : descriptor.mutators) {
        data.append(mutatorEnt.second->toBytes());
    }
    // zero-terminate the mutator list
    data.append(0);
    return allocate(data, QString("MutatorData_%1").arg(descriptor.internalName));
}

quint32 MutatorTable::writeTable(const std::vector<MapDescriptor> &descriptors) {
    QVector<quint32> table;
    for (int i=0; i<descriptors.size(); i++) {
        const MapDescriptor& descriptor = descriptors.at(i);
        if(descriptor.mutators.empty()) {
            table.append(0);
        } else {
            table.append(writeMutatorData(descriptor));
        }
    }
    return allocate(table, "MutatorTable");
}

void MutatorTable::writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &descriptors) {
    quint32 tableAddr = writeTable(descriptors);

    stream.device()->seek(addressMapper.boomToFileAddress(0x80412c88));
    // store the tableAddr so that csmm can retrieve it
    stream << tableAddr;
    // call 0x80412c8c for the subroutine
    stream.device()->seek(addressMapper.boomToFileAddress(0x80412c8c));
    auto routine = writeGetMutatorDataSubroutine(addressMapper, tableAddr);
    qDebug().noquote() << "Allocate (" + QString::number(routine.size()*4) + " bytes) at " + QString::number(addressMapper.boomStreetToStandard(0x80412c8c), 16) + " for GetMutatorDataSubroutine";
    for (auto word: routine) {
        stream << word;
    }
}

QVector<quint32> MutatorTable::writeGetMutatorDataSubroutine(const AddressMapper &addressMapper, quint32 tableAddr) {
    auto mapId = addressMapper.boomStreetToStandard(0x80552408);

    PowerPcAsm::Pair16Bit t = PowerPcAsm::make16bitValuePair(tableAddr);
    PowerPcAsm::Pair16Bit m = PowerPcAsm::make16bitValuePair(mapId);

    QVector<quint32> asm_;
    // precondition:  r3 - mutator type
    //         uses:  r3,r4,r5
    // postcondition: r3 - pointer to mutator data
    asm_.append(PowerPcAsm::lis(4, m.upper));           // \.
    asm_.append(PowerPcAsm::addi(4, 4, m.lower));       // |. r4 <- mapId
    asm_.append(PowerPcAsm::lwz(4, 0x0, 4));            // /.
    asm_.append(PowerPcAsm::cmpwi(4, 0));               // \. if(r4 < 0)
    asm_.append(PowerPcAsm::blt(8));                    // /.   goto retnone;
    asm_.append(PowerPcAsm::li(5, 2));                  // \.
    asm_.append(PowerPcAsm::slw(4, 4, 5));              // /. r4 <- r4 * 4
    asm_.append(PowerPcAsm::lis(5, t.upper));           // \.
    asm_.append(PowerPcAsm::addi(5, 5, t.lower));       // |. r5 <- tableAddr[mapId]
    asm_.append(PowerPcAsm::lwzx(5, 4, 5));             // /.
    asm_.append(PowerPcAsm::cmpwi(5, 0));               // \. if(r5 == NULL) {
    asm_.append(PowerPcAsm::bne(3));                    // |.
    int return_ = asm_.size();
    asm_.append(PowerPcAsm::li(3, 0));                  // |.   retnone: return r3 <- NULL
    asm_.append(PowerPcAsm::blr());                     // /. }
    int loop = asm_.size();
    asm_.append(PowerPcAsm::lha(4, 0x0, 5));            // |. r4 <- mutator type
    asm_.append(PowerPcAsm::cmpwi(4, 0));               // \. if(r4 == NULL) {
    asm_.append(PowerPcAsm::beq(asm_.size(), return_)); // /.  return r3 <- NULL
    asm_.append(PowerPcAsm::cmpw(3, 4));                // \. if(r3 == r4) {
    asm_.append(PowerPcAsm::bne(4));                    // |.
    asm_.append(PowerPcAsm::mr(3, 5));                  // |.
    asm_.append(PowerPcAsm::addi(3, 3, 0x4));           // |.   return r3 <- r5 + 0x4
    asm_.append(PowerPcAsm::blr());                     // /. }
    asm_.append(PowerPcAsm::lha(4, 0x2, 5));            // |. r4 <- mutator data size
    asm_.append(PowerPcAsm::add(4, 4, 4));              // \.
    asm_.append(PowerPcAsm::add(4, 4, 4));              // /. r4 <- 4*r4
    asm_.append(PowerPcAsm::addi(4, 4, 4));             // /. r4 <- r4+4
    asm_.append(PowerPcAsm::add(5, 4, 5));              // |. r5 <- r4 + r5
    asm_.append(PowerPcAsm::b(asm_.size(), loop));      // |. goto loop

    return asm_;
}

void MutatorTable::readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &, bool isVanilla) {
    if (!isVanilla) {
        for (auto &descriptor: mapDescriptors) {
            descriptor.mutators.clear();
            quint32 mutatorAddr;
            stream >> mutatorAddr;
            if (mutatorAddr != 0) {
                qint64 pos = stream.device()->pos();

                stream.device()->seek(mutatorAddr);
                auto mutator = Mutator::fromBytes(stream);
                while(!mutator.isNull()) {
                    mutator = Mutator::fromBytes(stream);
                    descriptor.mutators.emplace(mutatorTypeToString(mutator->type), mutator);
                }

                stream.device()->seek(pos);
            }
        }
    }
}

quint32 MutatorTable::readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x80412c88));
    quint32 addr;
    stream >> addr;
    return addr;
}

qint16 MutatorTable::readTableRowCount(QDataStream &, const AddressMapper &, bool) {
    return -1;
}

bool MutatorTable::readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) {
    stream.device()->seek(addressMapper.boomToFileAddress(0x80412c88));
    quint32 data; stream >> data;
    // 00000100
    return data == 0x00000100;
}
