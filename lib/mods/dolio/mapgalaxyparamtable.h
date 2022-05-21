#ifndef MAPGALAXYPARAMTABLE_H
#define MAPGALAXYPARAMTABLE_H

#include "doliotable.h"

class MapGalaxyParamTable : public virtual DolIOTable
{
public:
    static constexpr std::string_view MODID = "mapGalaxyParamTable";
    QString modId() const override { return MODID.data(); }
    void readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    quint32 writeTable(const std::vector<MapDescriptor> &descriptors);
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    qint16 readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
private:
    void readLoopingModeConfig(quint32 address, QDataStream &stream, MapDescriptor &mapDescriptor, const AddressMapper &addressMapper);
};

#endif // MAPGALAXYPARAMTABLE_H
