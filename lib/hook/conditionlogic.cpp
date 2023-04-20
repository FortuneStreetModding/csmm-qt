#include "conditionlogic.h"

#include <QDataStream>

ConditionLogic::ConditionLogic(OperationType conditionType, const YAML::Node &yaml) : AbstractCondition(conditionType, yaml) {
    conditions.clear();
    for(auto i=0;i<yaml.size();i++) {
        conditions.append(AbstractOperation::conditionFromYaml(yaml[i]));
    }
}

void ConditionLogic::toYaml__(YAML::Emitter& out) const {
    out << YAML::BeginSeq;
    for (auto &condition: conditions) {
        condition->toYaml(out);
    }
    out << YAML::EndSeq;
}

ConditionLogic::ConditionLogic(OperationType conditionType, QDataStream &stream) : AbstractCondition(conditionType, stream) {
    //stream >> number;
}

void ConditionLogic::toBytes__(QDataStream& stream) const {
    //stream << number;
}

bool ConditionLogic::operator==(const ConditionLogic &other) const {
    if(!AbstractCondition::operator==(other)) return false;
    if(type!=other.type) return false;
    if(conditions.size() != other.conditions.size()) return false;
    if(conditions != other.conditions) return false;
    return true;
};
