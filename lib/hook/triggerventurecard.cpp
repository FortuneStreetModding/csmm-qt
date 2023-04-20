#include "triggerventurecard.h"

#include <QDataStream>

TriggerVentureCard::TriggerVentureCard(const YAML::Node &yaml) : AbstractAction(TriggerVentureCardType, yaml) {
    number = yaml["number"].as<quint8>();
}

void TriggerVentureCard::toYaml__(YAML::Emitter& out) const {
    out << YAML::BeginMap;
    out << YAML::Key << "number" << YAML::Value << number;
    out << YAML::EndMap;
}

TriggerVentureCard::TriggerVentureCard(QDataStream &stream) : AbstractAction(TriggerVentureCardType, stream) {
    stream >> number;
}

void TriggerVentureCard::toBytes__(QDataStream& stream) const {
    stream << number;
}

bool TriggerVentureCard::operator==(const TriggerVentureCard &other) const {
    if(!AbstractAction::operator==(other)) return false;
    if(number != other.number) return false;
    return true;
};
