#include "rollshoppricemultiplier.h"

void RollShopPriceMultiplier::toYaml(YAML::Emitter& out) const {
    out << YAML::BeginMap;
    out << YAML::Key << "MaxRoll" << YAML::Value << maxRoll;
    out << YAML::EndMap;
}

RollShopPriceMultiplier::RollShopPriceMultiplier(const YAML::Node &yaml) : Mutator("RollShopPriceMultiplier") {
    maxRoll = yaml["MaxRoll"].as<quint32>();
}

bool RollShopPriceMultiplier::operator==(const Mutator &other) const {
    const RollShopPriceMultiplier *o = dynamic_cast<const RollShopPriceMultiplier *>(&other);
    if (o) {
        return maxRoll==o->maxRoll;
    }
    return false;
};
