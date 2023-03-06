#include "stockprice.h"

#include <QDataStream>

StockPrice::StockPrice(const YAML::Node &yaml) : Mutator(StockPriceType) {
    if(yaml["numerator"])
        numerator = yaml["numerator"].as<quint16>();
    if(yaml["denominator"])
        denominator = yaml["denominator"].as<quint16>();
    if(yaml["constant"])
        constant = yaml["constant"].as<qint16>();
}

void StockPrice::toYaml(YAML::Emitter& out) const {
    out << YAML::BeginMap;
    out << YAML::Key << "numerator" << YAML::Value << numerator;
    out << YAML::Key << "denominator" << YAML::Value << denominator;
    out << YAML::Key << "constant" << YAML::Value << constant;
    out << YAML::EndMap;
}

StockPrice::StockPrice(QDataStream &stream) : Mutator(StockPriceType) {
    stream >> numerator;
    stream >> denominator;
    stream >> constant;
}

void StockPrice::toBytes_(QDataStream& stream) const {
    stream << numerator;
    stream << denominator;
    stream << constant;
}

bool StockPrice::operator==(const Mutator &other) const {
    const StockPrice *o = dynamic_cast<const StockPrice *>(&other);
    if (o) {
        return numerator==o->numerator &&
               denominator==o->denominator &&
               constant==o->constant;
    }
    return false;
};
