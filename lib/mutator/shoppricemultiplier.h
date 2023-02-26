#ifndef SHOPPRICEMULTIPLIER_H
#define SHOPPRICEMULTIPLIER_H

#include "mutator.h"

struct ShopPriceMultiplier : Mutator {
    void toYaml(YAML::Emitter& out) const override;
    void toBytes_(QDataStream& data) const override;
    ShopPriceMultiplier(const YAML::Node &yaml);
    ShopPriceMultiplier(QDataStream &stream);
    bool operator==(const Mutator &other) const override;

    quint16 numerator;
    quint16 denominator;
};

#endif // SHOPPRICEMULTIPLIER_H
