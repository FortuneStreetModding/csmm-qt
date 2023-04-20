#ifndef ABSTRACTACTION_H
#define ABSTRACTACTION_H

#include <yaml-cpp/yaml.h>
#include <QException>
#include <QSharedPointer>

#include "abstractoperation.h"
#include "abstractcondition.h"

struct AbstractAction : AbstractOperation {
    bool oncePerTurn = true;
    QSharedPointer<AbstractCondition> when;

    virtual bool operator==(const AbstractAction &other) const;
    void toYaml(YAML::Emitter& out) const { AbstractOperation::toYaml(out); }
    void toBytes(QDataStream& stream) const { AbstractOperation::toBytes(stream); }
protected:
    AbstractAction(OperationType type, const YAML::Node &yaml);
    AbstractAction(OperationType type, QDataStream &stream);
    void toYaml_(YAML::Emitter& out) const override;
    void toBytes_(QDataStream& stream) const override;
    virtual void toYaml__(YAML::Emitter& out) const = 0;
    virtual void toBytes__(QDataStream& stream) const = 0;
};

#endif // ABSTRACTACTION_H
