#ifndef PYTHONBINDINGS_H
#define PYTHONBINDINGS_H

#include "pythoncasters.h"
#include "lib/mapdescriptor.h"

PYBIND11_MAKE_OPAQUE(std::array<bool, 128>);
PYBIND11_MAKE_OPAQUE(std::array<QString, 4>);
PYBIND11_MAKE_OPAQUE(std::vector<OriginPoint>);
PYBIND11_MAKE_OPAQUE(std::map<MusicType, MusicEntry>);

#endif // PYTHONBINDINGS_H
