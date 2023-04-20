#include "generichook.h"

#include <QDataStream>
#include <QMap>

static const QMap<QString, HookType> stringToHookTypes = {
    {"gameStartHook",  GameStartHookType},
    {"squareStopHook",  SquareStopHookType},
    {"turnEndHook",  TurnEndHookType}
};
QString hookTypeToString(HookType hookType) {
    return stringToHookTypes.key(hookType);
}
HookType stringToHookType(const QString &str) {
    return stringToHookTypes.value(str);
}

QSharedPointer<GenericHook> GenericHook::fromYaml(QString hookStr, const YAML::Node &yaml) {
    HookType type = stringToHookType(hookStr);
    if(type == InvalidHookType)
        throw HookException(QString("Invalid hook %1").arg(hookStr));

    QSharedPointer<GenericHook> hook(new GenericHook(type));
    hook->actions.clear();
    for(auto i=0;i<yaml.size();i++) {
        hook->actions.append(AbstractOperation::actionFromYaml(yaml[i]));
    }
    return hook;
}

void GenericHook::toYaml(YAML::Emitter &out) {
    out << YAML::BeginSeq;
    for(auto &action: actions) {
        action->toYaml(out);
    }
    out << YAML::EndSeq;
}

QSharedPointer<GenericHook> GenericHook::fromBytes(QDataStream &stream) {
    HookType type;
    stream >> type;
    if(type == InvalidHookType)
        throw HookException(QString("Invalid hook %1").arg(hookTypeToString(type)));
    QSharedPointer<GenericHook> hook(new GenericHook(type));
    quint16 size;
    stream >> size;

    qint64 endPos = stream.device()->pos() + size * 4;
    int limit = 100;
    hook->actions.clear();
    while(stream.device()->pos() < endPos) {
        hook->actions.append(AbstractOperation::actionFromBytes(stream));
        limit--;
        if(limit<=0)
            throw HookException(QString("Too many actions, hook %1 probably corrupt.").arg(hookTypeToString(type)));
    }
    return hook;
}

void GenericHook::toBytes(QDataStream &stream) {
    QByteArray body;
    QDataStream bodyDataStream(&body, QIODevice::WriteOnly);
    bodyDataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    for (auto &action: actions) {
        action->toBytes(bodyDataStream);
    }
    // add padding
    if((body.size()%4) != 0) {
        QByteArray padding(4 - (body.size()%4), '\0');
        bodyDataStream.writeRawData(padding, padding.size());
    }
    // save the hook type and the size of the body in the header
    stream << (quint16) type;
    stream << (quint16) (body.size() / 4);
    stream.writeRawData(body, body.size());
}

bool GenericHook::operator==(const GenericHook &other) const {
    const GenericHook *o = dynamic_cast<const GenericHook *>(&other);
    if (o) {
        if(type!=o->type)
            return false;
        if(actions.size() != o->actions.size())
            return false;
        if(actions != o->actions)
            return false;
        return true;
    }
    return false;
};
