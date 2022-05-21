#ifndef FORCESIMULATEDBUTTONPRESS_H
#define FORCESIMULATEDBUTTONPRESS_H

#include "dolio.h"

class ForceSimulatedButtonPress : public virtual DolIO
{
public:
    static constexpr std::string_view MODID = "forceSimulatedButtonPress";
    static constexpr std::string_view ADDRESS_FILE = "files/forceSimulatedButtonPress.dat"; // address of forceSimulatedButtonPress variable is stored here
    QString modId() const override { return MODID.data(); }
    void loadFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
private:
    QVector<quint32> writeUploadSimulatedButtonPress(const AddressMapper &addressMapper, quint32 routineStartAddress, quint32 returnAddr);
    quint32 forceSimulatedButtonPressAddr = 0;
};

#endif // FORCESIMULATEDBUTTONPRESS_H
