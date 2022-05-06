#ifndef INTERNALNAMETABLE_H
#define INTERNALNAMETABLE_H

#include "doliotable.h"

class InternalNameTable : public virtual DolIOTable
{
public:
    static constexpr std::string_view MODID = "internalNameTable";
    static constexpr std::string_view ADDRESS_FILE = "files/internalNameTable.dat";
    QString modId() const override { return MODID.data(); }
    void loadFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
    void readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    quint32 writeTable(const std::vector<MapDescriptor> &descriptors);
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    qint16 readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
private:
    quint32 internalNameDataAddr = 0;
};

#endif // INTERNALNAMETABLE_H
