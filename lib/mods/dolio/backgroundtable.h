#ifndef BACKGROUNDTABLE_H
#define BACKGROUNDTABLE_H

#include "doliotable.h"

class BackgroundTable : public virtual DolIOTable {
public:
    static constexpr std::string_view MODID = "backgroundTable";
    QString modId() const override { return MODID.data(); }
    QSet<QString> depends() const override { return {"allocateDescriptorCount"}; }
    void readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    quint32 writeTable(const std::vector<MapDescriptor> &descriptors);
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
};

#endif // BACKGROUNDTABLE_H
