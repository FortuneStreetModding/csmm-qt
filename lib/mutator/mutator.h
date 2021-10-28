#ifndef MUTATOR_H
#define MUTATOR_H

#include <QSharedPointer>
#include "yaml-cpp/yaml.h"

struct Mutator {
    const QString name;
    static QSharedPointer<Mutator> fromYaml(QString mutatorStr, const YAML::Node &yaml);
    virtual void toYaml(YAML::Emitter& out) const = 0;
    Mutator(const QString &name) : name(name) {};
    virtual ~Mutator();
    virtual bool operator==(const Mutator &other) const = 0;
    bool operator!=(const Mutator &other) const { return !(operator==(other)); };
};

#endif // MUTATOR_H
