#ifndef GENERICHOOK_H
#define GENERICHOOK_H

#include "abstractaction.h"
#include <QVector>

enum HookType : quint16 {
    InvalidHookType          = 0x00,
    GameStartHookType        = 0x01,
    SquareStopHookType       = 0x02,
    TurnEndHookType          = 0x03
};
QString hookTypeToString(HookType hookType);
HookType stringToHookType(const QString &str);

struct GenericHook {
    const HookType type;
    QVector<QSharedPointer<AbstractAction>> actions;

    static QSharedPointer<GenericHook> fromYaml(QString hookStr, const YAML::Node &yaml);
    static QSharedPointer<GenericHook> fromBytes(QDataStream &stream);
    void toBytes(QDataStream &stream);
    void toYaml(YAML::Emitter& out);
    GenericHook(const HookType type) : type(type) {};
    bool operator==(const GenericHook &other) const;
    bool operator!=(const GenericHook &other) const { return !(operator==(other)); };
};

class HookException : public QException, public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    const char *what() const noexcept override { return std::runtime_error::what(); }
    HookException(const QString &str) : std::runtime_error(str.toStdString()) {}
    void raise() const override { throw *this; }
    HookException *clone() const override { return new HookException(*this); }
};

#endif // GENERICHOOK_H
