#include "shoppricemultiplier.h"

#include <QDataStream>

void ShopPriceMultiplier::toYaml(YAML::Emitter& out) const {
    out << multiplier;
}

void ShopPriceMultiplier::toBytes(QVector<quint32>& data) const {
    data.append(multiplier);
}

ShopPriceMultiplier::ShopPriceMultiplier(const YAML::Node &yaml) : Mutator(ShopPriceMultiplierType) {
    multiplier = yaml.as<float>();
}

ShopPriceMultiplier::ShopPriceMultiplier(QDataStream &stream) : Mutator(ShopPriceMultiplierType) {
    stream >> multiplier;
}

bool ShopPriceMultiplier::operator==(const Mutator &other) const {
    const ShopPriceMultiplier *o = dynamic_cast<const ShopPriceMultiplier *>(&other);
    if (o) {
        return multiplier==o->multiplier;
    }
    return false;
};
