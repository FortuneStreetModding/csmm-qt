#include "rollshoppricemultiplier.h"

#include <QDataStream>

RollShopPriceMultiplier::RollShopPriceMultiplier(const YAML::Node &yaml, bool enabled) : Mutator(RollShopPriceMultiplierType, enabled) {
    maxRoll = yaml["maxRoll"].as<quint8>();
}

void RollShopPriceMultiplier::toYaml(YAML::Emitter& out) const {
    out << YAML::BeginMap;
    out << YAML::Key << "maxRoll" << YAML::Value << maxRoll;
    out << YAML::EndMap;
}

RollShopPriceMultiplier::RollShopPriceMultiplier(QDataStream &stream, bool enabled) : Mutator(RollShopPriceMultiplierType, enabled) {
    stream >> maxRoll;
}

void RollShopPriceMultiplier::toBytes_(QDataStream& stream) const {
    stream << maxRoll;
}

bool RollShopPriceMultiplier::operator==(const Mutator &other) const {
    const RollShopPriceMultiplier *o = dynamic_cast<const RollShopPriceMultiplier *>(&other);
    if (o) {
        return maxRoll==o->maxRoll;
    }
    return false;
};
