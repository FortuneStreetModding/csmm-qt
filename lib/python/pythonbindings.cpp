#include "pythonbindings.h"
#include <pybind11/embed.h>
#include <pybind11/stl_bind.h>
#include "lib/mapdescriptor.h"

PYBIND11_MAKE_OPAQUE(std::array<bool, 128>);

PYBIND11_EMBEDDED_MODULE(pycsmm, m) {
    pybind11::enum_<RuleSet>(m, "RuleSet")
            .value("Easy", Easy)
            .value("Standard", Standard);

    pybind11::class_<std::array<bool, 128>> ventureCardTable(m, "VentureCardTable");
    ventureCardTable.def(pybind11::init<>());
    pybind11::detail::vector_if_copy_constructible<std::array<bool, 128>, decltype(ventureCardTable)>(ventureCardTable);
    pybind11::detail::vector_if_equal_operator<std::array<bool, 128>, decltype(ventureCardTable)>(ventureCardTable);
    pybind11::detail::vector_accessor<std::array<bool, 128>, decltype(ventureCardTable)>(ventureCardTable);
    ventureCardTable.def(
            "__bool__",
            [](const std::array<bool, 128> &v) -> bool { return !v.empty(); },
            "Check whether the list is nonempty");
    ventureCardTable.def("__len__", &std::array<bool, 128>::size);

    pybind11::class_<MapDescriptor>(m, "MapDescriptor")
            .def(pybind11::init<>())
            .def_readwrite("mapSet", &MapDescriptor::mapSet)
            .def_readwrite("zone", &MapDescriptor::zone)
            .def_readwrite("order", &MapDescriptor::order)
            .def_readwrite("isPracticeBoard", &MapDescriptor::isPracticeBoard)
            .def_readwrite("unlockId", &MapDescriptor::unlockId)
            .def_readwrite("ruleSet", &MapDescriptor::ruleSet)
            .def_readwrite("initialCash", &MapDescriptor::initialCash)
            .def_readwrite("targetAmount", &MapDescriptor::targetAmount)
            .def_readwrite("baseSalary", &MapDescriptor::baseSalary)
            .def_readwrite("salaryIncrement", &MapDescriptor::salaryIncrement)
            .def_readwrite("maxDiceRoll", &MapDescriptor::maxDiceRoll)
            .def_readwrite("ventureCards", &MapDescriptor::ventureCards);
}
