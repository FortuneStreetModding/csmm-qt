#include "abstractcomparison.h"

#include <QDataStream>
#include <QMap>

AbstractComparison::AbstractComparison(OperationType type, const YAML::Node &yaml) : AbstractOperation(type) {
}

void AbstractComparison::toYaml_(YAML::Emitter& out) const {

}

AbstractComparison::AbstractComparison(OperationType type, QDataStream &stream) : AbstractOperation(type) {
}

void AbstractComparison::toBytes_(QDataStream& stream) const {
    QByteArray body;
    QDataStream bodyDataStream(&body, QIODevice::WriteOnly);
    bodyDataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    toBytes__(bodyDataStream);
    // save the action type and the size of the body in the header
    stream << (quint16) type;
    stream << (quint16) body.length();
    stream << body;
}

bool AbstractComparison::operator==(const AbstractComparison &other) const {
    if(!AbstractOperation::operator==(other)) return false;
    return true;
};
