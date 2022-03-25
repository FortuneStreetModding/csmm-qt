#ifndef DISPLAYMAPINRESULTS_H
#define DISPLAYMAPINRESULTS_H

#include "dolio.h"

class DisplayMapInResults : public virtual DolIO, public virtual ArcFileInterface {
public:
    static constexpr std::string_view MODID = "displayMapInResults";
    QString modId() const override { return MODID.data(); }
    QMap<QString, ModifyArcFunction> modifyArcFile() override;
    void readAsm(QDataStream &stream, const AddressMapper &addressMapper, QVector<MapDescriptor> &mapDescriptors) override;
protected:
    void writeAsm(QDataStream &stream, const AddressMapper &addressMapper, const QVector<MapDescriptor> &mapDescriptors) override;
};

#endif // DISPLAYMAPINRESULTS_H
