#include "mutator.h"

#include "rollshoppricemultiplier.h"
#include "shoppricemultiplier.h"
#include "stockpricemultiplier.h"

#include <QDataStream>
#include <QMap>

static const QMap<QString, MutatorType> stringToMutatorTypes = {
    {"rollShopPriceMultiplier",  RollShopPriceMultiplierType},
    {"shopPriceMultiplier",  ShopPriceMultiplierType},
    {"stockPriceMultiplier",  StockPriceMultiplierType},
};
QString mutatorTypeToString(MutatorType mutatorType) {
    return stringToMutatorTypes.key(mutatorType);
}
MutatorType stringToMutatorType(const QString &str) {
    return stringToMutatorTypes.value(str);
}

QSharedPointer<Mutator> Mutator::fromYaml(QString mutatorStr, const YAML::Node &yaml) {
    if(!stringToMutatorTypes.contains(mutatorStr))
        throw MutatorException(QString("Invalid mutator %1").arg(mutatorStr));
    auto mutatorType = stringToMutatorType(mutatorStr);
    switch(mutatorType) {
        case InvalidMutatorType: throw MutatorException(QString("Invalid mutator %1").arg(mutatorStr));
        case RollShopPriceMultiplierType: return QSharedPointer<Mutator>(new RollShopPriceMultiplier(yaml));
        case ShopPriceMultiplierType: return QSharedPointer<Mutator>(new ShopPriceMultiplier(yaml));
        case StockPriceMultiplierType: return QSharedPointer<Mutator>(new StockPriceMultiplier(yaml));
    }
    throw MutatorException(QString("Invalid mutator %1").arg(mutatorStr));
}

void Mutator::toBytes(QDataStream& stream) const {
    QByteArray body;
    QDataStream bodyDataStream(&body, QIODevice::WriteOnly);
    bodyDataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    toBytes_(bodyDataStream);
    if((body.size()%4) != 0) {
        QByteArray padding(4 - (body.size()%4), '\0');
        bodyDataStream.writeRawData(padding, padding.size());
    }
    // save the mutator type and the size of the body in the header
    stream << (quint16) type;
    stream << (quint16) (body.size() / 4);
    stream.writeRawData(body, body.size());
}

QSharedPointer<Mutator> Mutator::fromBytes(QDataStream &stream) {
    MutatorType mutatorType;
    stream >> mutatorType;
    quint16 size;
    stream >> size;
    switch(mutatorType) {
        case InvalidMutatorType: return QSharedPointer<Mutator>();
        case RollShopPriceMultiplierType: return QSharedPointer<Mutator>(new RollShopPriceMultiplier(stream));
        case ShopPriceMultiplierType: return QSharedPointer<Mutator>(new ShopPriceMultiplier(stream));
        case StockPriceMultiplierType: return QSharedPointer<Mutator>(new StockPriceMultiplier(stream));
    }
    throw MutatorException(QString("Invalid mutator %1").arg(mutatorType));
}

Mutator::~Mutator(void) {}
