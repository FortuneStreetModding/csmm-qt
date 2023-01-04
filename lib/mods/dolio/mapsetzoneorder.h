#ifndef MAPSETZONEORDER_H
#define MAPSETZONEORDER_H

#include "doliotable.h"

class MapSetZoneOrder : public virtual DolIOTable
{
public:
    static constexpr std::string_view MODID = "mapSetZoneOrder";
    QString modId() const override { return MODID.data(); }
    QSet<QString> depends() const override { return {"allocateDescriptorCount"}; }
    void readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    quint32 writeTable(const std::vector<MapDescriptor> &descriptors);
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    qint16 readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
private:
    QVector<quint32> writeSubroutineGetNumMapsInZone(const std::vector<MapDescriptor> &mapDescriptors);
    QVector<quint32> writeSubroutineGetMapsInZone(const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors, quint32 mapSetZoneOrderTable, quint32 entryAddr, quint32 returnAddr);
    void setVanillaMapSetZoneOrder(std::vector<MapDescriptor> &mapDescriptors);
};

#endif // MAPSETZONEORDER_H
