#ifndef STOCKPRICE_H
#define STOCKPRICE_H

#include "mutator.h"

struct StockPrice : Mutator {
    void toYaml(YAML::Emitter& out) const override;
    void toBytes_(QDataStream& data) const override;
    StockPrice(const YAML::Node &yaml);
    StockPrice(QDataStream &stream);
    bool operator==(const Mutator &other) const override;

    quint16 numerator = 1;
    quint16 denominator = 1;
    qint16 constant = 0;
};

#endif // STOCKPRICE_H
