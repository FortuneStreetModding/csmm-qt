#ifndef CUSTOMSHOPNAMES_H
#define CUSTOMSHOPNAMES_H

#include "doliotable.h"

class CustomShopNames : public virtual DolIOTable, public virtual UiMessageInterface
{
public:
    static constexpr std::string_view MODID = "customShopNames";
    static constexpr std::string_view MODID_CAPITAL = "customShopNamesCapital";
    CustomShopNames(bool isCapital) : isCapital(isCapital) {}
    QString modId() const override { return (isCapital ? MODID_CAPITAL : MODID).data(); }
    QSet<QString> depends() const override { return {"allocateDescriptorCount"}; }
    QMap<QString, LoadMessagesFunction> loadUiMessages() override;
    void allocateUiMessages(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
    QMap<QString, SaveMessagesFunction> saveUiMessages() override;
    void loadFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) override;
    void readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) override;
    bool readIsVanilla(QDataStream &stream, const AddressMapper &addressMapper) override;
    qint16 readTableRowCount(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
    quint32 readTableAddr(QDataStream &stream, const AddressMapper &addressMapper, bool isVanilla) override;
private:
    QString tableAddrFileName() { return isCapital ? "files/customShopNamesCapital.dat" : "files/customShopNames.dat"; }
    quint32 writeTable(const std::vector<MapDescriptor> &descriptors);
    QVector<quint32> getMsgIdSubroutine(const AddressMapper &mapper, quint32 routineStartAddr);
    bool isCapital;
    quint32 tableAddr = 0;
};

#endif // CUSTOMSHOPNAMES_H
