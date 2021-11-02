#ifndef ROLLSHOPPRICEMULTIPLIER_H
#define ROLLSHOPPRICEMULTIPLIER_H

#include "mutator.h"

struct RollShopPriceMultiplier : Mutator {
    void toYaml(YAML::Emitter& out) const override;
    void toBytes(QVector<quint32>& data) const override;
    RollShopPriceMultiplier(const YAML::Node &yaml);
    RollShopPriceMultiplier(QDataStream &stream);
    bool operator==(const Mutator &other) const override;

    qint16 maxRoll;
};

#endif // ROLLSHOPPRICEMULTIPLIER_H
