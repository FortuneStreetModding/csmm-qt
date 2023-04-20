#ifndef COMPARISONLOGIC_H
#define COMPARISONLOGIC_H

#include "abstractcomparison.h"

struct ComparisonLogic : AbstractComparison {
    QVector<QSharedPointer<AbstractComparison>> comparisons;

    bool operator==(const ComparisonLogic &other) const;
    ComparisonLogic(OperationType conditionType, const YAML::Node &yaml);
    ComparisonLogic(OperationType conditionType, QDataStream &stream);
protected:
    void toYaml__(YAML::Emitter& out) const override;
    void toBytes__(QDataStream& stream) const override;
};

#endif // COMPARISONLOGIC_H
