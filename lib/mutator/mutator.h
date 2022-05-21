#ifndef MUTATOR_H
#define MUTATOR_H

#include <QException>
#include <QSharedPointer>
#include "yaml-cpp/yaml.h"
#include "lib/addressmapping.h"

enum MutatorType : quint16 {
    NoneType                    = 0x00,
    RollAgainType               = 0x01,
    RollShopPriceMultiplierType = 0x02,
    ShopPriceMultiplierType     = 0x03
};
QString mutatorTypeToString(MutatorType mutatorType);
MutatorType stringToMutatorType(const QString &str);

struct Mutator {
    const MutatorType type;
    static QSharedPointer<Mutator> fromYaml(QString mutatorStr, const YAML::Node &yaml);
    static QSharedPointer<Mutator> fromBytes(QDataStream &stream);
    virtual void toYaml(YAML::Emitter& out) const = 0;
    QVector<quint32> toBytes() const;
    Mutator(const MutatorType type) : type(type) {};
    virtual ~Mutator();
    virtual bool operator==(const Mutator &other) const = 0;
    bool operator!=(const Mutator &other) const { return !(operator==(other)); };
protected:
    virtual void toBytes(QVector<quint32>& bytes) const = 0;
};

class MutatorException : public QException, public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    const char *what() const noexcept override { return std::runtime_error::what(); }
    MutatorException(const QString &str) : std::runtime_error(str.toStdString()) {}
    void raise() const override { throw *this; }
    MutatorException *clone() const override { return new MutatorException(*this); }
};

#endif // MUTATOR_H
