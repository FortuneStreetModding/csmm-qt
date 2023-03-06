#ifndef SHOPPRICE_H
#define SHOPPRICE_H

#include "mutator.h"

struct ShopPrice : Mutator {
    void toYaml(YAML::Emitter& out) const override;
    void toBytes_(QDataStream& data) const override;
    ShopPrice(const YAML::Node &yaml);
    ShopPrice(QDataStream &stream);
    bool operator==(const Mutator &other) const override;

    quint16 numerator = 1;
    quint16 denominator = 1;
    qint16 constant = 0;
    bool affectShopRank = true;
};

#endif // SHOPPRICE_H
