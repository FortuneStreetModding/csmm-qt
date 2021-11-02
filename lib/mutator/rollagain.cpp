#include "rollagain.h"

#include <QDataStream>

void RollAgain::toYaml(YAML::Emitter& out) const {
    out << YAML::BeginMap;
    out << YAML::Key << "WhenRollIs" << YAML::Value << whenRollIs;
    out << YAML::Key << "OnlyOnce" << YAML::Value << onlyOnce;
    out << YAML::Key << "RollAgainOptional" << YAML::Value << onlyOnce;
    out << YAML::EndMap;
}

void RollAgain::toBytes(QVector<quint32>& data) const {
    data.append(whenRollIs);
    data.append(onlyOnce);
    data.append(rollAgainOptional);
}

RollAgain::RollAgain(const YAML::Node &yaml) : Mutator(RollAgainType) {
    whenRollIs = yaml["WhenRollIs"].as<quint32>();
    onlyOnce = yaml["OnlyOnce"].as<bool>();
    rollAgainOptional = yaml["RollAgainOptional"].as<bool>();
}

RollAgain::RollAgain(QDataStream &stream) : Mutator(RollAgainType) {
    stream >> whenRollIs;
    stream >> onlyOnce;
    stream >> rollAgainOptional;
}

bool RollAgain::operator==(const Mutator &other) const {
    const RollAgain *o = dynamic_cast<const RollAgain *>(&other);
    if (o) {
        return whenRollIs==o->whenRollIs &&
               onlyOnce==o->onlyOnce &&
               rollAgainOptional==o->rollAgainOptional;
    }
    return false;
};
