#include "rollshoppricemultiplier.h"

#include <QDataStream>

void RollShopPriceMultiplier::toYaml(YAML::Emitter& out) const {
    out << YAML::BeginMap;
    out << YAML::Key << "MaxRoll" << YAML::Value << maxRoll;
    out << YAML::EndMap;
}

void RollShopPriceMultiplier::toBytes(QVector<quint32>& data) const {
    data.append(maxRoll);
}

RollShopPriceMultiplier::RollShopPriceMultiplier(const YAML::Node &yaml) : Mutator(RollShopPriceMultiplierType) {
    maxRoll = yaml["MaxRoll"].as<quint32>();
}

RollShopPriceMultiplier::RollShopPriceMultiplier(QDataStream &stream) : Mutator(RollShopPriceMultiplierType) {
    stream >> maxRoll;
}

bool RollShopPriceMultiplier::operator==(const Mutator &other) const {
    const RollShopPriceMultiplier *o = dynamic_cast<const RollShopPriceMultiplier *>(&other);
    if (o) {
        return maxRoll==o->maxRoll;
    }
    return false;
};
