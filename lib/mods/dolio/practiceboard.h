#ifndef PRACTICEBOARD_H
#define PRACTICEBOARD_H

#include "dolio.h"

class PracticeBoard : public virtual DolIO
{
public:
    static constexpr std::string_view MODID = "practiceBoard";
    QString modId() const override { return MODID.data(); }
    QSet<QString> after() const override { return { "mapSetZoneOrder" }; }
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
};

#endif // PRACTICEBOARD_H
