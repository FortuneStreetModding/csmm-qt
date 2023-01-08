#ifndef MAPDESCRIPTIONTABLE_H
#define MAPDESCRIPTIONTABLE_H

#include "doliotable.h"

class MapDescriptionTable : public virtual DolIOTable, public virtual UiMessageInterface
{
public:
    static constexpr std::string_view MODID = "mapDescriptionTable";
    QString modId() const override { return MODID.data(); }
    QSet<QString> after() const override { return { "mapOriginTable" }; }
    QSet<QString> depends() const override { return {"allocateDescriptorCount"}; }
    QMap<QString, LoadMessagesFunction> loadUiMessages() override;
    void allocateUiMessages(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
    QMap<QString, SaveMessagesFunction> saveUiMessages() override;
    void readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
    void loadFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    quint32 writeTable(const std::vector<MapDescriptor> &descriptors);
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    void readVanillaTable(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors);
};

#endif // MAPDESCRIPTIONTABLE_H
