#ifndef CHANGEMUSICONSWITCH_H
#define CHANGEMUSICONSWITCH_H

#include "dolio.h"

class ChangeMusicOnSwitch : public DolIO
{
public:
    static constexpr std::string_view MODID = "changeMusicOnSwitch";
    QString modId() const override { return MODID.data(); }
    QSet<QString> depends() const override { return { "musicTable" }; }
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
private:
    QVector<quint32> lookAtSwitchStateSubroutine(const AddressMapper &addressMapper, quint32 startAddr, quint32 oldSwitchStateAddr);
};

#endif // CHANGEMUSICONSWITCH_H
