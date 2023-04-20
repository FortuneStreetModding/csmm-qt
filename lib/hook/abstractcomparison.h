#ifndef ABSTRACTCOMPARISON_H
#define ABSTRACTCOMPARISON_H

#include <yaml-cpp/yaml.h>
#include <QException>
#include <QSharedPointer>
#include "abstractoperation.h"

struct AbstractComparison : AbstractOperation {
    virtual bool operator==(const AbstractComparison &other) const;
    void toYaml(YAML::Emitter& out) const { AbstractOperation::toYaml(out); }
    void toBytes(QDataStream& stream) const { AbstractOperation::toBytes(stream); }
protected:
    AbstractComparison(OperationType type, const YAML::Node &yaml);
    AbstractComparison(OperationType type, QDataStream &stream);
    void toYaml_(YAML::Emitter& out) const override;
    void toBytes_(QDataStream& stream) const override;
    virtual void toYaml__(YAML::Emitter& out) const = 0;
    virtual void toBytes__(QDataStream& stream) const = 0;
};

#endif // ABSTRACTCOMPARISON_H
