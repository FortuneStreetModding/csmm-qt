#ifndef DOLIOTABLE_H
#define DOLIOTABLE_H

#include "dolio.h"

class DolIOTable : public virtual DolIO {
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, std::vector<MapDescriptor> &mapDescriptors) override;
protected:
    virtual void readAsm(QDataStream &stream, std::vector<MapDescriptor> &mapDescriptors, const AddressMapper &addressMapper, bool isVanilla) = 0;
    virtual bool readIsVanilla(QDataStream & stream, const AddressMapper &addressMapper) = 0;
    virtual quint32 readTableAddr(QDataStream & stream, const AddressMapper &addressMapper, bool isVanilla) = 0;
};

#endif // DOLIOTABLE_H
