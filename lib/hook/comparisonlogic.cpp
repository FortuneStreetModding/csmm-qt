#include "comparisonlogic.h"

#include <QDataStream>

ComparisonLogic::ComparisonLogic(OperationType conditionType, const YAML::Node &yaml) : AbstractComparison(conditionType, yaml) {
    comparisons.clear();
    for(auto i=0;i<yaml.size();i++) {
        comparisons.append(AbstractOperation::comparisonFromYaml(yaml[i]));
    }
}

void ComparisonLogic::toYaml__(YAML::Emitter& out) const {
    out << YAML::BeginSeq;
    for (auto &comparison: comparisons) {
        comparison->toYaml(out);
    }
    out << YAML::EndSeq;
}

ComparisonLogic::ComparisonLogic(OperationType conditionType, QDataStream &stream) : AbstractComparison(conditionType, stream) {
    //stream >> number;
}

void ComparisonLogic::toBytes__(QDataStream& stream) const {
    //stream << number;
}

bool ComparisonLogic::operator==(const ComparisonLogic &other) const {
    if(!AbstractComparison::operator==(other)) return false;
    if(type!=other.type) return false;
    if(comparisons.size() != other.comparisons.size()) return false;
    if(comparisons != other.comparisons) return false;
    return true;
};
