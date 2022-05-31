#ifndef ARBITRARYNUMSWITCHSTATES_H
#define ARBITRARYNUMSWITCHSTATES_H

#include "dolio.h"

class ArbitraryNumSwitchStates : public DolIO
{
public:
    static constexpr std::string_view MODID = "arbitraryNumSwitchStates";
    QString modId() const override { return MODID.data(); };
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
};

#endif // ARBITRARYNUMSWITCHSTATES_H
