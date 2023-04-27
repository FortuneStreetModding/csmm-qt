#ifndef ARBITRARYNUMSWITCHSTATES_H
#define ARBITRARYNUMSWITCHSTATES_H

#include "dolio.h"

class ArbitraryNumSwitchStates : public DolIO
{
public:
    static constexpr std::string_view MODID = "arbitraryNumSwitchStates";
    QString modId() const override { return MODID.data(); };
    QSet<QString> depends() const override { return {"frbMapTable"}; }
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
private:
    void support3State(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors);
    void moveMapDataArray(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors, size_t maxStates);
    void increaseGameManagerStorage(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors, size_t maxStates);
    void expandedAnimations(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors, size_t maxStates);
    void increaseFrbBuffer(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors, size_t maxStates);
};

#endif // ARBITRARYNUMSWITCHSTATES_H
