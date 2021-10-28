#ifndef ROLLSHOPPRICEMULTIPLIER_H
#define ROLLSHOPPRICEMULTIPLIER_H

#include "mutator.h"

struct RollShopPriceMultiplier : Mutator {
    void toYaml(YAML::Emitter& out) const override;
    RollShopPriceMultiplier(const YAML::Node &yaml);
    bool operator==(const Mutator &other) const override;

    qint16 maxRoll;
};

#endif // ROLLSHOPPRICEMULTIPLIER_H
