#ifndef MAPSWITCHPARAMTABLE_H
#define MAPSWITCHPARAMTABLE_H

#include "doliotable.h"

class MapSwitchParamTable : public virtual DolIOTable
{
public:
    static constexpr std::string_view MODID = "mapSwitchParamTable";
    QString modId() const override { return MODID.data(); }
    QSet<QString> depends() const override { return {"allocateDescriptorCount"}; }
    void readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    quint32 writeTable(const std::vector<MapDescriptor> &descriptors);
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
private:
    void readRotationOriginPoints(quint32 address, QDataStream &stream, MapDescriptor &mapDescriptor, const AddressMapper &addressMapper);
};

#endif // MAPSWITCHPARAMTABLE_H
