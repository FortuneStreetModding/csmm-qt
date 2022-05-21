#ifndef WIFIFIX_H
#define WIFIFIX_H

#include "dolio.h"

class WifiFix : public virtual DolIO, public virtual UiMessageInterface
{
public:
    static constexpr std::string_view MODID = "wifiFix";
    QString modId() const override { return MODID.data(); }
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) override;
    QMap<QString, LoadMessagesFunction> loadUiMessages() override;
    QMap<QString, SaveMessagesFunction> freeUiMessages() override;
    void allocateUiMessages(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
    QMap<QString, SaveMessagesFunction> saveUiMessages() override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
private:
    QVector<quint32> writeSubroutineGetDefaultMapIdIntoR3(const AddressMapper &addressMapper, short defaultEasyMap, short defaultStandardMap, quint32 entryAddr, quint32 returnAddr);
    QVector<quint32> writeSubroutineGetDefaultMapIdIntoR4(short defaultEasyMap, short defaultStandardMap, quint32 entryAddr, quint32 returnAddr);
};

#endif // WIFIFIX_H
