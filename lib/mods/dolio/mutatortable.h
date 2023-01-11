#ifndef MUTATORTABLE_H
#define MUTATORTABLE_H

#include "doliotable.h"

class MutatorTable : public virtual DolIOTable
{
public:
    static constexpr std::string_view MODID = "mutatorTable";
    static constexpr std::string_view ADDRESS_FILE = "files/mutatorTable.dat";
    QString modId() const override { return MODID.data(); }
    QSet<QString> depends() const override { return {"allocateDescriptorCount"}; }
    void readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
    void loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    quint32 getMutatorTableStorageAddr() const;
    quint32 getMutatorTableRoutineAddr() const;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    quint32 writeTable(const std::vector<MapDescriptor> &descriptors);
    quint32 writeMutatorData(const MapDescriptor &descriptor);
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    QVector<quint32> writeGetMutatorDataSubroutine(const AddressMapper &addressMapper, quint32 tableAddr);
private:
    quint32 mutatorTableStorageAddr = 0;
};

#endif // MUTATORTABLE_H
