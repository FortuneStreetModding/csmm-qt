#ifndef PYTHONBINDINGS_H
#define PYTHONBINDINGS_H

#include "pythoncasters.h"
#include "lib/mapdescriptor.h"

PYBIND11_MAKE_OPAQUE(std::array<bool, 128>);
PYBIND11_MAKE_OPAQUE(std::array<QString, 4>);
PYBIND11_MAKE_OPAQUE(QVector<OriginPoint>);

#endif // PYTHONBINDINGS_H
