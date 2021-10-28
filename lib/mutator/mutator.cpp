#include "mutator.h"
#include "lib/patchprocess.h"

#include "rollagain.h"
#include "rollshoppricemultiplier.h"
#include "shoppricemultiplier.h"

QSharedPointer<Mutator> Mutator::fromYaml(QString mutatorStr, const YAML::Node &yaml) {
    if(mutatorStr == "RollAgain") return QSharedPointer<Mutator>(new RollAgain(yaml));
    if(mutatorStr == "RollShopPriceMultiplier") return QSharedPointer<Mutator>(new RollShopPriceMultiplier(yaml));
    if(mutatorStr == "ShopPriceMultiplier") return QSharedPointer<Mutator>(new ShopPriceMultiplier(yaml));
    throw PatchProcess::Exception(QString("Invalid mutator %1").arg(mutatorStr));
}

Mutator::~Mutator(void) {}
