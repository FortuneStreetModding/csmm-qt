#include "mutator.h"

#include "rollshoppricemultiplier.h"
#include "shopprice.h"
#include "stockprice.h"

#include <QDataStream>
#include <QIODevice>
#include <QMap>

static const QMap<QString, MutatorType> stringToMutatorTypes = {
    {"rollShopPriceMultiplier",  RollShopPriceMultiplierType},
    {"shopPrice",  ShopPriceType},
    {"stockPrice",  StockPriceType},
};
QString mutatorTypeToString(MutatorType mutatorType) {
    return stringToMutatorTypes.key(mutatorType);
}
MutatorType stringToMutatorType(const QString &str) {
    return stringToMutatorTypes.value(str, InvalidMutatorType);
}

QSharedPointer<Mutator> Mutator::fromYaml(QString mutatorStr, const YAML::Node &yaml) {
    if(!stringToMutatorTypes.contains(mutatorStr))
        throw MutatorException(QString("Invalid mutator %1").arg(mutatorStr));
    auto mutatorType = stringToMutatorType(mutatorStr);
    bool enabled = true;
    if(yaml["enabled"])
        enabled = yaml["enabled"].as<bool>();
    switch(mutatorType) {
        case InvalidMutatorType: throw MutatorException(QString("Invalid mutator %1").arg(mutatorStr));
        case RollShopPriceMultiplierType: return QSharedPointer<Mutator>(new RollShopPriceMultiplier(yaml, enabled));
        case ShopPriceType: return QSharedPointer<Mutator>(new ShopPrice(yaml, enabled));
        case StockPriceType: return QSharedPointer<Mutator>(new StockPrice(yaml, enabled));
    }
    throw MutatorException(QString("Invalid mutator %1").arg(mutatorStr));
}

void Mutator::toBytes(QDataStream& stream) const {
    QByteArray body;
    QDataStream bodyDataStream(&body, QIODevice::WriteOnly);
    bodyDataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    toBytes_(bodyDataStream);
    // add padding
    if((body.size()%4) != 0) {
        QByteArray padding(4 - (body.size()%4), '\0');
        bodyDataStream.writeRawData(padding, padding.size());
    }
    // save the mutator type and the size of the body in the header
    if(enabled) {
        stream << (quint16) (type | 0x8000);
    } else {
        stream << (quint16) type;
    }
    stream << (quint16) (body.size() / 4);
    stream.writeRawData(body, body.size());
}

QSharedPointer<Mutator> Mutator::fromBytes(QDataStream &stream) {
    quint16 mutatorTypeAndEnabled;
    stream >> mutatorTypeAndEnabled;
    bool enabled = (mutatorTypeAndEnabled & 0x8000) == 0x8000;
    MutatorType mutatorType = (MutatorType) (mutatorTypeAndEnabled & 0x7FFF);
    quint16 size;
    stream >> size;
    switch(mutatorType) {
        case InvalidMutatorType: return QSharedPointer<Mutator>();
        case RollShopPriceMultiplierType: return QSharedPointer<Mutator>(new RollShopPriceMultiplier(stream, enabled));
        case ShopPriceType: return QSharedPointer<Mutator>(new ShopPrice(stream, enabled));
        case StockPriceType: return QSharedPointer<Mutator>(new StockPrice(stream, enabled));
    }
    throw MutatorException(QString("Invalid mutator %1").arg(mutatorType));
}

Mutator::~Mutator(void) {}
