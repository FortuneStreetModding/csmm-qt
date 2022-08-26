#ifndef RIIVOLUTION_H
#define RIIVOLUTION_H

#include "lib/addressmapping.h"
#include <QDir>
#include <QException>

namespace Riivolution {

bool validateRiivolutionName(const QString &riivolutionName);
void write(const QDir &vanilla, const QDir &fullPatchDir, const AddressMapper &addressMapper, const QString &discId, const QString &riivolutionName);

class Exception : public QException, public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    const char *what() const noexcept override { return std::runtime_error::what(); }
    Exception(const QString &str) : std::runtime_error(str.toStdString()) {}
    void raise() const override { throw *this; }
    Exception *clone() const override { return new Exception(*this); }
};

}

#endif // RIIVOLUTION_H
