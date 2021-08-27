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

    /**
     * @param versionAddress the virtual address
     * @return whether the standard virtual address maps to a valid file address
     */
    bool canConvertToFileAddress(quint32 versionAddress) const;
    /**
     * Converts the given standard virtual address to a file address.
     * @param versionAddress the virtual address
     * @return the file address
     */
    qint32 toFileAddress(quint32 versionAddress) const;
    /**
     * Converts the given Boom Street virtual address to a file address.
     * @param versionAddress the virtual address
     * @return the file address
     */
    qint32 boomToFileAddress(quint32 versionAddress) const;
    /**
     * Converts the given file address to a standard virtual address.
     * @param fileAddress the file address
     * @return the virtual address
     */
    quint32 fileAddressToStandardVirtualAddress(qint64 fileAddress) const;
    /**
     * Converts the given Boom Street virtual address to a standard virtual address.
     * @param boomAddresss the Boom Street address
     * @return the Fortune Street address
     */
    quint32 boomStreetToStandard(quint32 boomAddress) const;
    /**
     * Converts the given standard virtual address to a Boom Street virtual address.
     * @param standardAddress the standard address
     * @return the Boom Street address
     */
    quint32 standardToBoomStreet(quint32 standardAddress) const;
    /**
     * @param boomAddresss the Boom Street address
     * @return whether the Boom Street address maps to a valid standard address
     */
    bool canConvertBoomStreetToStandard(quint32 boomAddress) const;
private:
    AddressSectionMapper fileMapper;
    AddressSectionMapper versionMapper;
};


#endif // ADDRESSMAPPING_H
