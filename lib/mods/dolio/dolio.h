#ifndef DOLIO_H
#define DOLIO_H

#include <QDataStream>
#include "lib/freespacemanager.h"
#include "lib/mapdescriptor.h"
#include "lib/mods/csmmmod.h"

/**
 * @brief Legacy class for operations on main.dol, retrofitted as a CSMM mod.
 */
class DolIO : public virtual CSMMMod, public virtual GeneralInterface {
public:
    void loadFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    void saveFiles(const QString &root, GameInstance *gameInstance, const ModListType &modList) override;
    void write(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors, FreeSpaceManager &freeSpaceManager);
    virtual void readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) = 0;
    virtual ~DolIO();
protected:
    virtual void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const std::vector<MapDescriptor> &mapDescriptors) = 0;
    quint32 allocate(const QByteArray &data, const QString &purpose, bool reuse = true);
    quint32 allocate(const QByteArray &data, const char *purpose, bool reuse = true);
    quint32 allocate(const QVector<quint32> &words, const QString &purpose, bool reuse = true);
    quint32 allocate(const QVector<quint16> &words, const QString &purpose, bool reuse = true);
    template<class Iter>
    std::enable_if_t<
        std::is_arithmetic_v<typename std::iterator_traits<Iter>::value_type>,
        quint32
    > allocate(Iter it0, Iter it1, const QString &purpose, bool reuse = true) {
        QByteArray data;
        QDataStream dataStream(&data, QIODevice::WriteOnly);
        for (auto it = it0; it != it1; ++it) {
            dataStream << *it;
        }
        return allocate(data, purpose, reuse);
    };
    quint32 allocate(const QString &str, bool reuse = true);
    /**
     * @param stream the main.dol stream
     * @param fn function taking the address to allocate at (or 0 if called just for calculating allocated memory size) and returning the memory content
     * @param purpose the reason for allocating the memory
     * @return the subroutine start addr
     */
    quint32 writeSubroutine(QDataStream &stream, const std::function<QVector<quint32>(quint32)> &fn, const QString &purpose);
    const ModListType &modList();
    QString resolveAddressToString(quint32 virtualAddress, QDataStream &stream, const AddressMapper &addressMapper);
private:
    FreeSpaceManager *fsmPtr;
    QDataStream *streamPtr;
    const AddressMapper *mapperPtr;
    const ModListType *modListPtr;
};

#endif // DOLIO_H
