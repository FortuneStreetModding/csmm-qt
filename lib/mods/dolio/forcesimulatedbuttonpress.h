#ifndef FORCESIMULATEDBUTTONPRESS_H
#define FORCESIMULATEDBUTTONPRESS_H

#include "dolio.h"

class ForceSimulatedButtonPress : public virtual DolIO
{
public:
    static constexpr std::string_view MODID = "forceSimulatedButtonPress";
    QString modId() const override { return MODID.data(); }
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
private:
    QVector<quint32> writeUploadSimulatedButtonPress(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 returnAddr);
};

#endif // FORCESIMULATEDBUTTONPRESS_H
