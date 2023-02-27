#include "stockpricemultiplier.h"

#include <QDataStream>

StockPriceMultiplier::StockPriceMultiplier(const YAML::Node &yaml) : Mutator(StockPriceMultiplierType) {
    numerator = yaml["numerator"].as<quint16>();
    denominator = yaml["denominator"].as<quint16>();
}

void StockPriceMultiplier::toYaml(YAML::Emitter& out) const {
    out << YAML::BeginMap;
    out << YAML::Key << "numerator" << YAML::Value << numerator;
    out << YAML::Key << "denominator" << YAML::Value << denominator;
    out << YAML::EndMap;
}

StockPriceMultiplier::StockPriceMultiplier(QDataStream &stream) : Mutator(StockPriceMultiplierType) {
    stream >> numerator;
    stream >> denominator;
}

void StockPriceMultiplier::toBytes_(QDataStream& stream) const {
    stream << numerator;
    stream << denominator;
}

bool StockPriceMultiplier::operator==(const Mutator &other) const {
    const StockPriceMultiplier *o = dynamic_cast<const StockPriceMultiplier *>(&other);
    if (o) {
        return numerator==o->numerator &&
               denominator==o->denominator;
    }
    return false;
};
