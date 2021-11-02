#ifndef SHOPPRICEMULTIPLIER_H
#define SHOPPRICEMULTIPLIER_H

#include "mutator.h"

struct ShopPriceMultiplier : Mutator {
    void toYaml(YAML::Emitter& out) const override;
    void toBytes(QVector<quint32>& data) const override;
    ShopPriceMultiplier(const YAML::Node &yaml);
    ShopPriceMultiplier(QDataStream &stream);
    bool operator==(const Mutator &other) const override;

    float multiplier;
};

#endif // SHOPPRICEMULTIPLIER_H
