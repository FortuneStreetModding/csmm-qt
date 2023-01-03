#ifndef NAMEDDISTRICTS_H
#define NAMEDDISTRICTS_H

#include "doliotable.h"

class NamedDistricts : public virtual DolIOTable, public virtual UiMessageInterface, public virtual ArcFileInterface
{
public:
    static constexpr std::string_view MODID = "namedDistricts";
    QString modId() const override { return MODID.data(); }
    QSet<QString> depends() const override { return {"allocateDescriptorCount"}; }
    QMap<QString, LoadMessagesFunction> loadUiMessages() override;
    virtual void allocateUiMessages(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
    QMap<QString, SaveMessagesFunction> saveUiMessages() override;
    void readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
    QMap<QString, ModifyArcFunction> modifyArcFile() override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    quint32 writeTable(const std::vector<MapDescriptor> &descriptors);
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    qint16 readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
};

#endif // NAMEDDISTRICTS_H
