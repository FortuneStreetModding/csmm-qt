#ifndef STOCKPRICEMULTIPLIER_H
#define STOCKPRICEMULTIPLIER_H

#include "mutator.h"

struct StockPriceMultiplier : Mutator {
    void toYaml(YAML::Emitter& out) const override;
    void toBytes_(QDataStream& data) const override;
    StockPriceMultiplier(const YAML::Node &yaml);
    StockPriceMultiplier(QDataStream &stream);
    bool operator==(const Mutator &other) const override;

    quint16 numerator;
    quint16 denominator;
};

#endif // STOCKPRICEMULTIPLIER_H
