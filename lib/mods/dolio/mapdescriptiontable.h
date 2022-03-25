#ifndef MAPDESCRIPTIONTABLE_H
#define MAPDESCRIPTIONTABLE_H

#include "doliotable.h"

class MapDescriptionTable : public virtual DolIOTable, public virtual UiMessageInterface
{
public:
    static constexpr std::string_view MODID = "mapDescriptionTable";
    QString modId() const override { return MODID.data(); }
    QSet<QString> after() const override { return { "mapOriginTable" }; }
    QMap<QString, LoadMessagesFunction> loadUiMessages() override;
    QMap<QString, SaveMessagesFunction> freeUiMessages() override;
    void allocateUiMessages(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
    QMap<QString, SaveMessagesFunction> saveUiMessages() override;
    void readAsm(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
    quint32 writeTable(const QVector<MapDescriptor> &descriptors);
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    qint16 readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    void readVanillaTable(QDataStream &stream, QVector<MapDescriptor> &mapDescriptors);
};

#endif // MAPDESCRIPTIONTABLE_H
