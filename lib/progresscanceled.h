
#ifndef PROGRESSCANCELED_H
#define PROGRESSCANCELED_H

#include <QException>

class ProgressCanceled : public QException, public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    const char *what() const noexcept override { return std::runtime_error::what(); }
    ProgressCanceled(const QString &str) : std::runtime_error(str.toStdString()) {}
    void raise() const override { throw *this; }
    ProgressCanceled *clone() const override { return new ProgressCanceled(*this); }
};

#endif // PROGRESSCANCELED_H
