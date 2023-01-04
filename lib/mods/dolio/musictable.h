#ifndef MUSICTABLE_H
#define MUSICTABLE_H

#include "doliotable.h"

class MusicTable : public virtual DolIOTable, public virtual GeneralInterface
{
public:
    static constexpr std::string_view MODID = "musicTable";
    QString modId() const override { return MODID.data(); }
    QSet<QString> after() const override { return { "defaultMiscPatches" }; }
    QSet<QString> depends() const override { return {"allocateDescriptorCount"}; }
    void loadFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
    void readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    quint32 writeBgmTable(const std::vector<MapDescriptor> &descriptors);
    quint32 writeMeTable(const std::vector<MapDescriptor> &descriptors);
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    qint16 readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;

private:
    QVector<quint32> writeSubroutineReplaceBgmId(const AddressMapper &addressMapper, quint32 tableAddr, quint32 entryAddr, quint32 returnContinueAddr, quint32 returnBgmReplacedAddr);
};

#endif // MUSICTABLE_H
