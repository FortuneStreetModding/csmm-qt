#include "abstractoperation.h"
#include "triggerventurecard.h"

#include <QDataStream>
#include <QMap>
#include <QDebug>

#include "comparisonlogic.h"
#include "conditionlogic.h"
#include "triggerventurecard.h"

static const QMap<QString, OperationType> stringToOperationTypes = {
    {"triggerVentureCard", TriggerVentureCardType},
    {"all", ConditionAllType},
    {"any", ConditionAnyType},
    {"one", ConditionOneType},
    {"none", ConditionNoneType},
};
QString operationTypeToString(OperationType operationType) {
    return stringToOperationTypes.key(operationType);
}
OperationType stringToOperationType(const QString &str) {
    return stringToOperationTypes.value(str);
}

OperationType getOperationType(const QString abstractOperationStr) {
    if(!stringToOperationTypes.contains(abstractOperationStr))
        throw OperationException(QString("Unknown operation %1").arg(abstractOperationStr));
    auto operationType = stringToOperationType(abstractOperationStr);
    if(operationType == InvalidOperationType)
        throw OperationException(QString("Invalid operation %1").arg(abstractOperationStr));
    return operationType;
}

QString toStr(const YAML::Node &yaml) {
    YAML::Emitter out;
    out << yaml;
    QString output(out.c_str());
    return output;
}

// --- --- --- ---
// --- Actions ---
QSharedPointer<AbstractAction> actionFromYaml(const OperationType operationType, const YAML::Node &yaml) {
    switch(operationType) {
        case TriggerVentureCardType: return QSharedPointer<AbstractAction>(new TriggerVentureCard(yaml));
    default:
        throw OperationException(QString("Unknown action %1").arg(operationTypeToString(operationType)));
    }
}
QSharedPointer<AbstractAction> actionFromBytes(const OperationType operationType, QDataStream &stream) {
    switch(operationType) {
        case TriggerVentureCardType: return QSharedPointer<AbstractAction>(new TriggerVentureCard(stream));
    default:
        throw OperationException(QString("Unknown action %1").arg(operationTypeToString(operationType)));
    }
}
// --- Comparisons ---
QSharedPointer<AbstractComparison> comparisonFromYaml(const OperationType operationType, const YAML::Node &yaml) {
    switch(operationType) {
        case ComparisonAllType:  return QSharedPointer<AbstractComparison>(new ComparisonLogic(ComparisonAllType, yaml));
        case ComparisonAnyType:  return QSharedPointer<AbstractComparison>(new ComparisonLogic(ComparisonAnyType, yaml));
        case ComparisonOneType:  return QSharedPointer<AbstractComparison>(new ComparisonLogic(ComparisonOneType, yaml));
        case ComparisonNoneType: return QSharedPointer<AbstractComparison>(new ComparisonLogic(ComparisonNoneType, yaml));
    default:
        throw OperationException(QString("Unknown comparison %1").arg(operationTypeToString(operationType)));
    }
}
QSharedPointer<AbstractComparison> comparisonFromBytes(const OperationType operationType, QDataStream &stream) {
    switch(operationType) {
        case ComparisonAllType:  return QSharedPointer<AbstractComparison>(new ComparisonLogic(ComparisonAllType, stream));
        case ComparisonAnyType:  return QSharedPointer<AbstractComparison>(new ComparisonLogic(ComparisonAnyType, stream));
        case ComparisonOneType:  return QSharedPointer<AbstractComparison>(new ComparisonLogic(ComparisonOneType, stream));
        case ComparisonNoneType: return QSharedPointer<AbstractComparison>(new ComparisonLogic(ComparisonNoneType, stream));
    default:
        throw OperationException(QString("Unknown comparison %1").arg(operationTypeToString(operationType)));
    }
}
// --- Conditions ---
QSharedPointer<AbstractCondition> conditionFromYaml(const OperationType operationType, const YAML::Node &yaml) {
    switch(operationType) {
        case ConditionAllType:  return QSharedPointer<AbstractCondition>(new ConditionLogic(ConditionAllType, yaml));
        case ConditionAnyType:  return QSharedPointer<AbstractCondition>(new ConditionLogic(ConditionAnyType, yaml));
        case ConditionOneType:  return QSharedPointer<AbstractCondition>(new ConditionLogic(ConditionOneType, yaml));
        case ConditionNoneType: return QSharedPointer<AbstractCondition>(new ConditionLogic(ConditionNoneType, yaml));
    default:
        throw OperationException(QString("Unknown condition %1").arg(operationTypeToString(operationType)));
    }
}
QSharedPointer<AbstractCondition> conditionFromBytes(const OperationType operationType, QDataStream &stream) {
    switch(operationType) {
        case ConditionAllType:  return QSharedPointer<AbstractCondition>(new ConditionLogic(ConditionAllType, stream));
        case ConditionAnyType:  return QSharedPointer<AbstractCondition>(new ConditionLogic(ConditionAnyType, stream));
        case ConditionOneType:  return QSharedPointer<AbstractCondition>(new ConditionLogic(ConditionOneType, stream));
        case ConditionNoneType: return QSharedPointer<AbstractCondition>(new ConditionLogic(ConditionNoneType, stream));
    default:
        throw OperationException(QString("Unknown condition %1").arg(operationTypeToString(operationType)));
    }
}
// --- --- --- ---
QSharedPointer<AbstractAction> AbstractOperation::actionFromYaml(const YAML::Node &yaml) {
    auto singleton = yaml.begin();
    QString abstractOperationStr = QString::fromStdString(singleton->first.as<std::string>());
    auto operationType = getOperationType(abstractOperationStr);
    return ::actionFromYaml(operationType, singleton->second);
}
QSharedPointer<AbstractComparison> AbstractOperation::comparisonFromYaml(const YAML::Node &yaml) {
    auto singleton = yaml.begin();
    QString abstractOperationStr = QString::fromStdString(singleton->first.as<std::string>());
    auto operationType = getOperationType(abstractOperationStr);
    return ::comparisonFromYaml(operationType, singleton->second);
}
QSharedPointer<AbstractCondition> AbstractOperation::conditionFromYaml(const YAML::Node &yaml) {
    auto singleton = yaml.begin();
    QString abstractOperationStr = QString::fromStdString(singleton->first.as<std::string>());
    auto operationType = getOperationType(abstractOperationStr);
    return ::conditionFromYaml(operationType, singleton->second);
}
QSharedPointer<AbstractOperation> AbstractOperation::fromYaml(const YAML::Node &yaml) {
    auto singleton = yaml.begin();
    QString abstractOperationStr = QString::fromStdString(singleton->first.as<std::string>());
    auto operationType = getOperationType(abstractOperationStr);
    try { return qSharedPointerDynamicCast<AbstractOperation>(::actionFromYaml(operationType, singleton->second)); }  catch (OperationException) { }
    //try { return qSharedPointerDynamicCast<AbstractOperation>(::comparisonFromYaml(operationType, singleton->second)); }  catch (OperationException) { }
    try { return qSharedPointerDynamicCast<AbstractOperation>(::conditionFromYaml(operationType, singleton->second)); }  catch (OperationException) { }
    throw OperationException(QString("Invalid hook operation %1").arg(operationTypeToString(operationType)));
}

void AbstractOperation::toYaml(YAML::Emitter& out) const {
    out << YAML::Key << operationTypeToString(type).toStdString();
    out << YAML::Value; toYaml_(out);
}

QSharedPointer<AbstractAction> AbstractOperation::actionFromBytes(QDataStream &stream) {
    OperationType operationType;
    stream >> operationType;
    return ::actionFromBytes(operationType, stream);
}
QSharedPointer<AbstractComparison> AbstractOperation::comparisonFromBytes(QDataStream &stream) {
    OperationType operationType;
    stream >> operationType;
    return ::comparisonFromBytes(operationType, stream);
}
QSharedPointer<AbstractCondition> AbstractOperation::conditionFromBytes(QDataStream &stream) {
    OperationType operationType;
    stream >> operationType;
    return ::conditionFromBytes(operationType, stream);
}
QSharedPointer<AbstractOperation> AbstractOperation::fromBytes(QDataStream &stream) {
    OperationType operationType;
    stream >> operationType;
    try { return qSharedPointerDynamicCast<AbstractOperation>(::actionFromBytes(operationType, stream)); }  catch (OperationException) { }
    //try { return qSharedPointerDynamicCast<AbstractOperation>(::comparisonFromBytes(operationType, stream)); }  catch (OperationException) { }
    try { return qSharedPointerDynamicCast<AbstractOperation>(::conditionFromBytes(operationType, stream)); }  catch (OperationException) { }
    throw OperationException(QString("Invalid hook operation %1").arg(operationTypeToString(operationType)));
}

void AbstractOperation::toBytes(QDataStream& stream) const {
    QByteArray body;
    QDataStream bodyDataStream(&body, QIODevice::WriteOnly);
    bodyDataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    toBytes_(bodyDataStream);
    // save the operation type and the size of the body in the header
    stream << (quint8) type;
    stream << (quint8) body.length();
    stream << body;
}


bool AbstractOperation::operator==(const AbstractOperation &other) const {
    return type==other.type;
};
