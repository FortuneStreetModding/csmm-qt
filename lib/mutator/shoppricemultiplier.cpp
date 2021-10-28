#include "shoppricemultiplier.h"

void ShopPriceMultiplier::toYaml(YAML::Emitter& out) const {
    out << multiplier;
}

ShopPriceMultiplier::ShopPriceMultiplier(const YAML::Node &yaml) : Mutator("ShopPriceMultiplier") {
    multiplier = yaml.as<float>();
}

bool ShopPriceMultiplier::operator==(const Mutator &other) const {
    const ShopPriceMultiplier *o = dynamic_cast<const ShopPriceMultiplier *>(&other);
    if (o) {
        return multiplier==o->multiplier;
    }
    return false;
};
