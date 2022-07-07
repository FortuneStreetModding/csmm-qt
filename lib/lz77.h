#ifndef LZ77_H
#define LZ77_H

#include <QDataStream>
#include <QException>

namespace LZ77 {

constexpr quint8 LZ77 = 0x1;

struct Archive {
    quint8 algorithm;
    quint8 isExtended;
    quint32 size;
};

std::pair<Archive, QByteArray> extract(QDataStream &src);
void compress(const QByteArray &src, QDataStream &dest, bool isExtended);

class Exception : public QException, public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    const char *what() const noexcept override { return std::runtime_error::what(); }
    Exception(const QString &str) : std::runtime_error(str.toStdString()) {}
    void raise() const override { throw *this; }
    Exception *clone() const override { return new Exception(*this); }
};

}

#endif // LZ77_H
