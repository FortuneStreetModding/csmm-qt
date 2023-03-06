#include "shoppricemultiplier.h"

#include <QDataStream>

ShopPriceMultiplier::ShopPriceMultiplier(const YAML::Node &yaml) : Mutator(ShopPriceMultiplierType) {
    if(yaml["numerator"])
        numerator = yaml["numerator"].as<quint16>();
    if(yaml["denominator"])
        denominator = yaml["denominator"].as<quint16>();
    if(yaml["constant"])
        constant = yaml["constant"].as<qint16>();
    if(yaml["affectShopRank"])
        affectShopRank = yaml["affectShopRank"].as<bool>();
}

void ShopPriceMultiplier::toYaml(YAML::Emitter& out) const {
    out << YAML::BeginMap;
    out << YAML::Key << "numerator" << YAML::Value << numerator;
    out << YAML::Key << "denominator" << YAML::Value << denominator;
    out << YAML::Key << "constant" << YAML::Value << constant;
    out << YAML::Key << "affectShopRank" << YAML::Value << affectShopRank;
    out << YAML::EndMap;
}

ShopPriceMultiplier::ShopPriceMultiplier(QDataStream &stream) : Mutator(ShopPriceMultiplierType) {
    stream >> numerator;
    stream >> denominator;
    stream >> constant;
    stream >> affectShopRank;
}

void ShopPriceMultiplier::toBytes_(QDataStream& stream) const {
    stream << numerator;
    stream << denominator;
    stream << constant;
    stream << affectShopRank;
}

bool ShopPriceMultiplier::operator==(const Mutator &other) const {
    const ShopPriceMultiplier *o = dynamic_cast<const ShopPriceMultiplier *>(&other);
    if (o) {
        return numerator==o->numerator &&
               denominator==o->denominator &&
               constant==o->constant &&
               affectShopRank==o->affectShopRank;
    }
    return false;
};
