#include "addressmapping.h"

bool AddressSection::containsVirtualAddress(qint64 virtualAddress) const {
    return offsetBeg <= virtualAddress && virtualAddress <= offsetEnd;
}
qint64 AddressSection::toFileAddress(qint64 virtualAddress) const {
    return virtualAddress - fileDelta;
}
bool AddressSection::containsFileAddress(qint64 fileAddress) const {
    return containsVirtualAddress(toVirtualAddress(fileAddress));
}
qint64 AddressSection::toVirtualAddress(qint64 fileAddress) const {
    return fileAddress + fileDelta;
}

AddressSectionMapper::AddressSectionMapper(const QVector<AddressSection> &sectionsVal) : sections(sectionsVal) {}
bool AddressSectionMapper::sectionAvailable(QString sectionName) const {
    for (auto &section: sections) {
        if (QString::compare(section.sectionName, sectionName, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}
const AddressSection *AddressSectionMapper::findSection(qint64 address) const {
    for (auto &section: sections) {
        if (section.containsVirtualAddress(address)) {
            return &section;
        }
    }
    return nullptr;
}
qint64 AddressSectionMapper::map(qint64 address) const {
    auto sectionPtr = findSection(address);
    if (sectionPtr) {
        return (int)sectionPtr->toFileAddress(address);
    }
    return -1;
}
qint64 AddressSectionMapper::inverseMap(qint64 address) const {
    for (auto &section: sections) {
        if (section.containsFileAddress(address)) {
            return (int)section.toVirtualAddress(address);
        }
    }
    return -1;
}

AddressMapper::AddressMapper(const AddressSectionMapper &fileMapperVal) : fileMapper(fileMapperVal) {}
void AddressMapper::setVersionMapper(const AddressSectionMapper &versionMapperVal, GameVersion versionVal) {
    versionMapper = versionMapperVal;
    version = versionVal;
}
bool AddressMapper::canConvertToFileAddress(quint32 versionAddress) const {
    return fileMapper.findSection(versionAddress);
}
qint32 AddressMapper::toFileAddress(quint32 versionAddress) const {
    return fileMapper.map(versionAddress);
}
qint32 AddressMapper::boomToFileAddress(quint32 versionAddress) const {
    return toFileAddress(boomStreetToStandard(versionAddress));
}
quint32 AddressMapper::fileAddressToStandardVirtualAddress(qint64 fileAddress) const {
    return fileMapper.inverseMap(fileAddress);
}
quint32 AddressMapper::boomStreetToStandard(quint32 boomAddress) const {
    return versionMapper.map(boomAddress);
}
quint32 AddressMapper::standardToBoomStreet(quint32 standardAddress) const {
    return versionMapper.inverseMap(standardAddress);
}
bool AddressMapper::canConvertBoomStreetToStandard(quint32 boomAddress) const {
    return versionMapper.findSection(boomAddress);
}

GameVersion AddressMapper::getVersion() const
{
    return version;
}
