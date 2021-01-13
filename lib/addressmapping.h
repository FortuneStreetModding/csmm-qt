#ifndef ADDRESSMAPPING_H
#define ADDRESSMAPPING_H

#include <limits>
#include <QString>
#include <QVector>

struct AddressSection {
    qint64 offsetBeg = std::numeric_limits<qint64>::min();
    qint64 offsetEnd = std::numeric_limits<qint64>::max();
    qint64 fileDelta = 0;
    QString sectionName = "Identity";
    bool containsVirtualAddress(qint64 virtualAddress) const;
    qint64 toFileAddress(qint64 virtualAddress) const;
    bool containsFileAddress(qint64 fileAddress) const;
    qint64 toVirtualAddress(qint64 fileAddress) const;
};

class AddressSectionMapper {
public:
    AddressSectionMapper(const QVector<AddressSection> &sectionsVal = {});
    bool sectionAvailable(QString sectionName) const;
    const AddressSection *findSection(qint64 address) const;
    qint64 map(qint64 address) const;
    qint64 inverseMap(qint64 address) const;
private:
    QVector<AddressSection> sections;
};

class AddressMapper {
public:
    AddressMapper(const AddressSectionMapper &fileMapperVal = AddressSectionMapper());
    void setVersionMapper(const AddressSectionMapper &versionMapperVal);
    bool canConvertToFileAddress(quint32 versionAddress) const;
    qint32 toFileAddress(quint32 versionAddress) const;
    qint32 boomToFileAddress(quint32 versionAddress) const;
    quint32 fileAddressToStandardVirtualAddress(qint64 fileAddress) const;
    quint32 boomStreetToStandard(quint32 boomAddress) const;
    quint32 standardToBoomStreet(quint32 standardAddress) const;
    bool canConvertBoomStreetToStandard(quint32 boomAddress) const;
private:
    AddressSectionMapper fileMapper;
    AddressSectionMapper versionMapper;
};


#endif // ADDRESSMAPPING_H
