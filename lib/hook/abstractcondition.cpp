#include "abstractcondition.h"

#include <QDataStream>
#include <QMap>

AbstractCondition::AbstractCondition(OperationType type, const YAML::Node &yaml) : AbstractOperation(type) {
}

void AbstractCondition::toYaml_(YAML::Emitter& out) const {

}

AbstractCondition::AbstractCondition(OperationType type, QDataStream &stream) : AbstractOperation(type) {
}

void AbstractCondition::toBytes_(QDataStream& stream) const {
    QByteArray body;
    QDataStream bodyDataStream(&body, QIODevice::WriteOnly);
    bodyDataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    toBytes__(bodyDataStream);
    // save the action type and the size of the body in the header
    stream << (quint16) type;
    stream << (quint16) body.length();
    stream << body;
}

bool AbstractCondition::operator==(const AbstractCondition &other) const {
    if(!AbstractOperation::operator==(other)) return false;
    return true;
};
