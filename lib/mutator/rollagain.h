#ifndef ROLLAGAIN_H
#define ROLLAGAIN_H

#include "mutator.h"

struct RollAgain : Mutator {
    void toYaml(YAML::Emitter& out) const override;
    void toBytes(QVector<quint32>& data) const override;
    RollAgain(const YAML::Node &yaml);
    RollAgain(QDataStream &stream);
    bool operator==(const Mutator &other) const override;

    qint16 whenRollIs;
    bool onlyOnce;
    bool rollAgainOptional;
};

#endif // ROLLAGAIN_H
