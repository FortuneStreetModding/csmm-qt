#ifndef DOLIOMUTATOR_H
#define DOLIOMUTATOR_H

#include "dolio.h"

class DolIOMutator : public virtual DolIO {
public:
    QSet<QString> depends() const override;
    QSet<QString> after() const override;
protected:
    static CSMMModHolder mutatorTableMod(const ModListType &modList);
    static quint32 mutatorTableStorageAddr(const ModListType &modList);
};

#endif // DOLIOMUTATOR_H
