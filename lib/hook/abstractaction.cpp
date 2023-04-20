#include "abstractaction.h"

#include <QDataStream>
#include <QMap>

AbstractAction::AbstractAction(OperationType type, const YAML::Node &yaml) : AbstractOperation(type) {
    if(yaml["oncePerTurn"])
        oncePerTurn = yaml["oncePerTurn"].as<bool>();
    if(yaml["when"])
        when = AbstractOperation::conditionFromYaml(yaml["when"]);
}

void AbstractAction::toYaml_(YAML::Emitter& out) const {
    out << YAML::BeginMap;
    out << YAML::Key << "oncePerTurn" << YAML::Value << oncePerTurn;
    out << YAML::Key << "when" << YAML::Value; when->toYaml(out);
    out << YAML::EndMap;
}

AbstractAction::AbstractAction(OperationType type, QDataStream &stream) : AbstractOperation(type) {
    stream >> oncePerTurn;
    when->toBytes(stream);
}

void AbstractAction::toBytes_(QDataStream& stream) const {
    QByteArray body;
    QDataStream bodyDataStream(&body, QIODevice::WriteOnly);
    bodyDataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    toBytes__(bodyDataStream);
    // save the action type and the size of the body in the header
    stream << (quint16) type;
    stream << (quint16) body.length();
    // TODO: when
    stream << body;
}

bool AbstractAction::operator==(const AbstractAction &other) const {
    if(!AbstractOperation::operator==(other)) return false;
    if(oncePerTurn != other.oncePerTurn) return false;
    if(when != other.when) return false;
    return true;
};
