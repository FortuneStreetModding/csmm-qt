#ifndef CONDITIONLOGIC_H
#define CONDITIONLOGIC_H

#include "abstractcondition.h"

struct ConditionLogic : AbstractCondition {
    QVector<QSharedPointer<AbstractCondition>> conditions;

    bool operator==(const ConditionLogic &other) const;
    ConditionLogic(OperationType conditionType, const YAML::Node &yaml);
    ConditionLogic(OperationType conditionType, QDataStream &stream);
protected:
    void toYaml__(YAML::Emitter& out) const override;
    void toBytes__(QDataStream& stream) const override;
};

#endif // CONDITIONLOGIC_H
