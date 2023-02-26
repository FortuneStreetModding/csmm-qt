#include "shoppricemultiplier.h"

#include <QDataStream>

ShopPriceMultiplier::ShopPriceMultiplier(const YAML::Node &yaml) : Mutator(ShopPriceMultiplierType) {
    numerator = yaml["numerator"].as<quint16>();
    denominator = yaml["denominator"].as<quint16>();
}

void ShopPriceMultiplier::toYaml(YAML::Emitter& out) const {
    out << YAML::BeginMap;
    out << YAML::Key << "numerator" << YAML::Value << numerator;
    out << YAML::Key << "denominator" << YAML::Value << denominator;
    out << YAML::EndMap;
}

ShopPriceMultiplier::ShopPriceMultiplier(QDataStream &stream) : Mutator(ShopPriceMultiplierType) {
    stream >> numerator;
    stream >> denominator;
}

void ShopPriceMultiplier::toBytes_(QDataStream& stream) const {
    stream << numerator;
    stream << denominator;
}

bool ShopPriceMultiplier::operator==(const Mutator &other) const {
    const ShopPriceMultiplier *o = dynamic_cast<const ShopPriceMultiplier *>(&other);
    if (o) {
        return numerator==o->numerator &&
               denominator==o->denominator;
    }
    return false;
};
