#ifndef TRIGGERVENTURECARD_H
#define TRIGGERVENTURECARD_H

#include "abstractaction.h"

struct TriggerVentureCard : AbstractAction {
    qint8 number;

    bool operator==(const TriggerVentureCard &other) const;
    TriggerVentureCard(const YAML::Node &yaml);
    TriggerVentureCard(QDataStream &stream);
protected:
    void toYaml__(YAML::Emitter& out) const override;
    void toBytes__(QDataStream& stream) const override;
};

#endif // TRIGGERVENTURECARD_H
