#include "doliomutator.h"
#include "mutatortable.h"

QSet<QString> DolIOMutator::depends() const {
    return { MutatorTable::MODID.data() };
}

CSMMModHolder DolIOMutator::mutatorTableMod(const ModListType &modList) {
    auto it = std::find_if(modList.begin(), modList.end(), [](const auto &mod) { return mod->modId() == MutatorTable::MODID.data(); });
    return it != modList.end() ? *it : CSMMModHolder();
}

quint32 DolIOMutator::mutatorTableStorageAddr(const ModListType &modList)
{
    auto mutatorMod = mutatorTableMod(modList);
    auto castedMutatorMod = std::dynamic_pointer_cast<const MutatorTable>(mutatorMod.modHandle());
    if (!castedMutatorMod) {
        throw ModException("mutator mod not found!");
    }
    return castedMutatorMod->getMutatorTableStorageAddr();
}
