#include "stockpricemultiplier.h"

#include <QDataStream>

StockPriceMultiplier::StockPriceMultiplier(const YAML::Node &yaml) : Mutator(StockPriceMultiplierType) {
    if(yaml["numerator"])
        numerator = yaml["numerator"].as<quint16>();
    if(yaml["denominator"])
        denominator = yaml["denominator"].as<quint16>();
    if(yaml["constant"])
        constant = yaml["constant"].as<qint16>();
}

void StockPriceMultiplier::toYaml(YAML::Emitter& out) const {
    out << YAML::BeginMap;
    out << YAML::Key << "numerator" << YAML::Value << numerator;
    out << YAML::Key << "denominator" << YAML::Value << denominator;
    out << YAML::Key << "constant" << YAML::Value << constant;
    out << YAML::EndMap;
}

StockPriceMultiplier::StockPriceMultiplier(QDataStream &stream) : Mutator(StockPriceMultiplierType) {
    stream >> numerator;
    stream >> denominator;
    stream >> constant;
}

void StockPriceMultiplier::toBytes_(QDataStream& stream) const {
    stream << numerator;
    stream << denominator;
    stream << constant;
}

bool StockPriceMultiplier::operator==(const Mutator &other) const {
    const StockPriceMultiplier *o = dynamic_cast<const StockPriceMultiplier *>(&other);
    if (o) {
        return numerator==o->numerator &&
               denominator==o->denominator &&
               constant==o->constant;
    }
    return false;
};
