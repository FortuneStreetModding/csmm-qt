#ifndef ABSTRACTCONDITION_H
#define ABSTRACTCONDITION_H

#include <yaml-cpp/yaml.h>
#include <QException>
#include <QSharedPointer>
#include "abstractoperation.h"

struct AbstractCondition : AbstractOperation {
    virtual bool operator==(const AbstractCondition &other) const;
    void toYaml(YAML::Emitter& out) const { AbstractOperation::toYaml(out); }
    void toBytes(QDataStream& stream) const { AbstractOperation::toBytes(stream); }
protected:
    AbstractCondition(OperationType type, const YAML::Node &yaml);
    AbstractCondition(OperationType type, QDataStream &stream);
    void toYaml_(YAML::Emitter& out) const override;
    void toBytes_(QDataStream& stream) const override;
    virtual void toYaml__(YAML::Emitter& out) const = 0;
    virtual void toBytes__(QDataStream& stream) const = 0;
};

#endif // ABSTRACTCONDITION_H
