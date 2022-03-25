#include "mutator.h"
#include "lib/importexportutils.h"

#include "rollagain.h"
#include "rollshoppricemultiplier.h"
#include "shoppricemultiplier.h"

#include <QDataStream>

static const QMap<QString, MutatorType> stringToMutatorTypes = {
    {"rollAgain", RollAgainType},
    {"rollShopPriceMultiplier",  RollShopPriceMultiplierType},
    {"shopPriceMultiplier",  ShopPriceMultiplierType}
};

QString mutatorTypeToString(MutatorType mutatorType) {
    return stringToMutatorTypes.key(mutatorType);
}
MutatorType stringToMutatorType(const QString &str) {
    return stringToMutatorTypes.value(str);
}

QSharedPointer<Mutator> Mutator::fromYaml(QString mutatorStr, const YAML::Node &yaml) {
    if(!stringToMutatorTypes.contains(mutatorStr))
        throw ImportExportUtils::Exception(QString("Invalid mutator %1").arg(mutatorStr));
    switch(stringToMutatorType(mutatorStr)) {
        case NoneType: throw ImportExportUtils::Exception(QString("Invalid mutator %1").arg(mutatorStr));
        case RollAgainType: return QSharedPointer<Mutator>(new RollAgain(yaml));
        case RollShopPriceMultiplierType: return QSharedPointer<Mutator>(new RollShopPriceMultiplier(yaml));
        case ShopPriceMultiplierType: return QSharedPointer<Mutator>(new ShopPriceMultiplier(yaml));
    }
    throw ImportExportUtils::Exception(QString("Invalid mutator %1").arg(mutatorStr));
}

QVector<quint32> Mutator::toBytes() const {
    QVector<quint32> body;
    toBytes(body);
    // save the mutator type and the size of the body in the header
    quint32 headerWord = type;
    headerWord <<= 16;
    headerWord |= body.length();

    QVector<quint32> data;
    data.append(headerWord);
    data.append(body);
    return data;
}

QSharedPointer<Mutator> Mutator::fromBytes(QDataStream &stream) {
    MutatorType type;
    stream >> type;
    quint16 size;
    stream >> size;
    switch(type) {
        case NoneType: return QSharedPointer<Mutator>();
        case RollAgainType: return QSharedPointer<Mutator>(new RollAgain(stream));
        case RollShopPriceMultiplierType: return QSharedPointer<Mutator>(new RollShopPriceMultiplier(stream));
        case ShopPriceMultiplierType: return QSharedPointer<Mutator>(new ShopPriceMultiplier(stream));
    }
    throw ImportExportUtils::Exception(QString("Invalid mutator %1").arg(type));
}

Mutator::~Mutator(void) {}
