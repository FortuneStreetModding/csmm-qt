#ifndef PYTHONBINDINGS_H
#define PYTHONBINDINGS_H

#include "pythoncasters.h"
#include "lib/mapdescriptor.h"

PYBIND11_MAKE_OPAQUE(std::array<bool, 128>);
PYBIND11_MAKE_OPAQUE(std::array<QString, 4>);
PYBIND11_MAKE_OPAQUE(std::vector<OriginPoint>);
PYBIND11_MAKE_OPAQUE(std::map<MusicType, MusicEntry>);
PYBIND11_MAKE_OPAQUE(std::array<Character, 3>);
PYBIND11_MAKE_OPAQUE(std::map<QString, QString>);
PYBIND11_MAKE_OPAQUE(std::vector<QString>);
PYBIND11_MAKE_OPAQUE(std::map<QString, std::vector<QString>>);
PYBIND11_MAKE_OPAQUE(std::vector<quint32>);

#endif // PYTHONBINDINGS_H
