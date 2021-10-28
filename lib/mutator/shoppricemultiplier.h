#ifndef SHOPPRICEMULTIPLIER_H
#define SHOPPRICEMULTIPLIER_H

#include "mutator.h"

struct ShopPriceMultiplier : Mutator {
    void toYaml(YAML::Emitter& out) const override;
    ShopPriceMultiplier(const YAML::Node &yaml);
    bool operator==(const Mutator &other) const override;

    float multiplier;
};

#endif // SHOPPRICEMULTIPLIER_H
