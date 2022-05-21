#ifndef MAPICONTABLE_H
#define MAPICONTABLE_H

#include "doliotable.h"

class MapIconTable : public virtual DolIOTable, public virtual ArcFileInterface
{
public:
    static constexpr std::string_view MODID = "mapIconTable";
    QString modId() const override { return MODID.data(); }
    QSet<QString> after() const override { return { "backgroundTable", "mapOriginTable" }; }
    void readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;\
    QMap<QString, ModifyArcFunction> modifyArcFile() override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    qint16 readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;

    QMap<QString, quint32> writeIconStrings(const std::vector<MapDescriptor> &mapDescriptors);
    quint32 writeIconTable(const QMap<QString, quint32> &mapIcons, QMap<QString, quint32> &iconTableMap);
    quint32 writeMapIconPointerTable(const std::vector<MapDescriptor> &mapDescriptors, const QMap<QString, quint32> &iconTableMap);

    QString resolveAddressAddressToString(quint32 virtualAddressAddress, QDataStream &stream, const AddressMapper &addressMapper);
private:
    QVector<quint32> writeSubroutineInitMapIdsForMapIcons(const AddressMapper &addressMapper, quint32 entryAddr);
    QVector<quint32> writeSubroutineMakeNoneMapIconsInvisible(const AddressMapper &addressMapper, quint32 entryAddr, quint32 returnContinueAddr, quint32 returnMakeInvisibleAddr);
    QVector<quint32> writeSubroutineSkipMapUnlockCheck(quint32 entryAddr, quint32 returnContinueAddr, quint32 returnSkipMapUnlockedCheckAddr);
};

#endif // MAPICONTABLE_H
