#include "shopprice.h"

#include <QDataStream>

ShopPrice::ShopPrice(const YAML::Node &yaml, bool enabled) : Mutator(ShopPriceType, enabled) {
    if(yaml["numerator"])
        numerator = yaml["numerator"].as<quint16>();
    if(yaml["denominator"])
        denominator = yaml["denominator"].as<quint16>();
    if(yaml["constant"])
        constant = yaml["constant"].as<qint16>();
    if(yaml["affectShopRank"])
        affectShopRank = yaml["affectShopRank"].as<bool>();
}

void ShopPrice::toYaml(YAML::Emitter& out) const {
    out << YAML::BeginMap;
    out << YAML::Key << "numerator" << YAML::Value << numerator;
    out << YAML::Key << "denominator" << YAML::Value << denominator;
    out << YAML::Key << "constant" << YAML::Value << constant;
    out << YAML::Key << "affectShopRank" << YAML::Value << affectShopRank;
    out << YAML::EndMap;
}

ShopPrice::ShopPrice(QDataStream &stream, bool enabled) : Mutator(ShopPriceType, enabled) {
    stream >> numerator;
    stream >> denominator;
    stream >> constant;
    stream >> affectShopRank;
}

void ShopPrice::toBytes_(QDataStream& stream) const {
    stream << numerator;
    stream << denominator;
    stream << constant;
    stream << affectShopRank;
}

bool ShopPrice::operator==(const Mutator &other) const {
    const ShopPrice *o = dynamic_cast<const ShopPrice *>(&other);
    if (o) {
        return numerator==o->numerator &&
               denominator==o->denominator &&
               constant==o->constant &&
               affectShopRank==o->affectShopRank;
    }
    return false;
};
