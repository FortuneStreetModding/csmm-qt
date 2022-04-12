#ifndef EVENTSQUAREMOD_H
#define EVENTSQUAREMOD_H

#include "dolio.h"

class EventSquareMod : public virtual DolIO, public virtual UiMessageInterface
{
public:
    static constexpr std::string_view MODID = "eventSquare";
    QString modId() const override { return MODID.data(); }
    QMap<QString, LoadMessagesFunction> loadUiMessages() override;
    QMap<QString, SaveMessagesFunction> freeUiMessages() override;
    void allocateUiMessages(const QString &root, GameInstance &gameInstance, const ModListType &modList) override;
    QMap<QString, SaveMessagesFunction> saveUiMessages() override;
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
private:
    QVector<quint32> writeGetDescriptionForCustomSquareRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress);
    QVector<quint32> writeGetModelForCustomSquareRoutine(quint8 register_textureType, quint8 register_squareType);
    QVector<quint32> writeGetTextureForCustomSquareRoutine(const AddressMapper &addressMapper,  quint32 routineStartAddress, quint32 eventSquareFormatAddr, quint32 eventSquareTextureAddr);
    QVector<quint32> writeGetMinimapTileIdForCustomSquareRoutine(const AddressMapper &addressMapper,  quint32 routineStartAddress);
    QVector<quint32> writeFontCharacterIdModifierRoutine(const AddressMapper &addressMapper, quint32 routineStartAddress);
    QVector<quint32> writeMinimapTileIdHandler(const AddressMapper &addressMapper, quint32 routineStartAddress);
    QVector<quint32> writeProcStopEventSquareRoutine(const AddressMapper &addressMapper, quint32 forceVentureCardVariable, quint32 routineStartAddress);
    QVector<quint32> writeSubroutineForceFetchFakeVentureCard(quint32 fakeVentureCard);
    quint32 eventSquareId;
    quint32 freeParkingId;
    quint32 freeParkingDescId;
};

#endif // EVENTSQUAREMOD_H