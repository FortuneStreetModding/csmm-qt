#ifndef ABSTRACTOPERATION_H
#define ABSTRACTOPERATION_H

#include <yaml-cpp/yaml.h>
#include <QException>
#include <QSharedPointer>

enum OperationType : quint8 {
    InvalidOperationType        = 0x00,
    TriggerVentureCardType      = 0x01,
    ConditionAllType            = 0x02,
    ConditionAnyType            = 0x03,
    ConditionOneType            = 0x04,
    ConditionNoneType           = 0x05
};
QString operationTypeToString(OperationType OperationType);
OperationType stringToOperationType(const QString &str);

struct AbstractCondition;
struct AbstractComparison;
struct AbstractAction;

struct AbstractOperation {
    const OperationType type;
    static QSharedPointer<AbstractOperation> fromYaml(const YAML::Node &yaml);
    static QSharedPointer<AbstractCondition> conditionFromYaml(const YAML::Node &yaml);
    static QSharedPointer<AbstractComparison> comparisonFromYaml(const YAML::Node &yaml);
    static QSharedPointer<AbstractAction> actionFromYaml(const YAML::Node &yaml);

    static QSharedPointer<AbstractOperation> fromBytes(QDataStream &stream);
    static QSharedPointer<AbstractCondition> conditionFromBytes(QDataStream &stream);
    static QSharedPointer<AbstractComparison> comparisonFromBytes(QDataStream &stream);
    static QSharedPointer<AbstractAction> actionFromBytes(QDataStream &stream);
    void toYaml(YAML::Emitter& out) const;
    void toBytes(QDataStream& stream) const;
    virtual bool operator==(const AbstractOperation &other) const;
    bool operator!=(const AbstractOperation &other) const { return !(operator==(other)); };
protected:
    virtual void toYaml_(YAML::Emitter& out) const = 0;
    virtual void toBytes_(QDataStream& stream) const = 0;
    AbstractOperation(const OperationType type) : type(type) {};
};

class OperationException : public QException, public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    const char *what() const noexcept override { return std::runtime_error::what(); }
    OperationException(const QString &str) : std::runtime_error(str.toStdString()) {}
    void raise() const override { throw *this; }
    OperationException *clone() const override { return new OperationException(*this); }
};

#endif // ABSTRACTOPERATION_H
