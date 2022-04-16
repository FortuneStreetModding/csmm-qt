#include "pythonbindings.h"
#include <pybind11/embed.h>
#include <pybind11/stl_bind.h>

static std::ostream &operator<<(std::ostream &stream, const QString &str) {
    return stream << str.toStdString();
}

template<class ...Args>
static void defineContainerRepr(Args&&... args) {
}

template<class Cont>
static auto defineContainerRepr(pybind11::class_<Cont> &cls, const std::string &name)
    -> decltype(std::declval<std::ostringstream>() << std::declval<Cont>().front(), void()) {
    cls.def("__repr__", [name](const Cont &v) {
        std::ostringstream ss;
        ss << name << "[";
        for (auto it = v.begin(); it != v.end(); ++it) {
            ss << *it;
            if (std::next(it) != v.end()) {
                ss << ", ";
            }
        }
        ss << "]";
        return ss.str();
    }, "Return the canonical string representation of this list.");
}

template<class Cont>
static void bindConstContainerOps(pybind11::class_<Cont> &cls, const std::string &name) {
    using ConstTRef = decltype(std::declval<const Cont>().front());
    cls.def(
            "__bool__",
            [](const Cont &v) -> bool { return !v.empty(); },
            "Check whether the list is nonempty");
    cls.def("__len__", &Cont::size);
    cls.def("__contains__", [](const Cont &v, ConstTRef x) { return std::find(v.begin(), v.end(), x) != v.end(); });
    defineContainerRepr(cls, name);
}

template<class T, size_t N, class ...Args>
static void bindStdArray(pybind11::handle h, const std::string &name, Args &&...args) {
    pybind11::class_<std::array<T, N>> cls(h, name.c_str(), std::forward(args)...);
    cls.def(pybind11::init<>());
    pybind11::detail::vector_if_copy_constructible<std::array<T, N>, decltype(cls)>(cls);
    pybind11::detail::vector_accessor<std::array<T, N>, decltype(cls)>(cls);
    bindConstContainerOps(cls, name);
}

PYBIND11_EMBEDDED_MODULE(pycsmm, m) {
    pybind11::enum_<RuleSet>(m, "RuleSet")
            .value("Easy", Easy)
            .value("Standard", Standard);

    pybind11::enum_<BoardTheme>(m, "BoardTheme")
            .value("DragonQuest", DragonQuest)
            .value("Mario", Mario);

    pybind11::enum_<BgmId> bgmIdEnum(m, "BgmId");
    auto &bgmIdMap = Bgm::bgmIdMapping();
    for (auto it=bgmIdMap.begin(); it!=bgmIdMap.end(); ++it) {
        bgmIdEnum.value(it.key().toUtf8(), it.value());
    }

    pybind11::enum_<MusicType> musicTypeEnum(m, "MusicType");
    auto &musicTypeMap = Music::stringToMusicTypeMapping();
    for (auto it=musicTypeMap.begin(); it!=musicTypeMap.end(); ++it) {
        musicTypeEnum.value(it.key().toUtf8(), it.value());
    }

    pybind11::enum_<LoopingMode>(m, "LoopingMode")
            .value("None", None)
            .value("Vertical", Vertical)
            .value("Both", Both);

    pybind11::class_<OriginPoint>(m, "OriginPoint")
            .def(pybind11::init([](float x, float y) { return OriginPoint{x, y}; }), pybind11::arg("x"),  pybind11::arg("y"))
            .def_readwrite("x", &OriginPoint::x)
            .def_readwrite("y", &OriginPoint::y);

    pybind11::class_<MusicEntry>(m, "MusicEntry")
            .def_readwrite("brstmBaseFilename", &MusicEntry::brstmBaseFilename)
            .def_readwrite("volume", &MusicEntry::volume)
            .def_readwrite("brsarIndex", &MusicEntry::brsarIndex)
            .def_readwrite("brstmFileSize", &MusicEntry::brstmFileSize);

    bindStdArray<bool, 128>(m, "VentureCardTable");

    bindStdArray<QString, 4>(m, "FrbFiles");

    pybind11::bind_vector<std::vector<OriginPoint>>(m, "OriginPoints");

    pybind11::bind_map<std::map<MusicType, MusicEntry>>(m, "MusicEntryTable");

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
            .def_readwrite("ventureCards", &MapDescriptor::ventureCards)
            .def_readwrite("frbFiles", &MapDescriptor::frbFiles)
            .def_readwrite("switchRotationOrigins", &MapDescriptor::switchRotationOrigins)
            .def_readwrite("theme", &MapDescriptor::theme)
            .def_readwrite("background", &MapDescriptor::background)
            .def_readwrite("bgmId", &MapDescriptor::bgmId)
            .def_readwrite("mapIcon", &MapDescriptor::mapIcon)
            .def_readwrite("music", &MapDescriptor::music)
            .def_readwrite("loopingMode", &MapDescriptor::loopingMode)
            .def_readwrite("loopingModeRadius", &MapDescriptor::loopingModeRadius)
            .def_readwrite("loopingModeHorizontalPadding", &MapDescriptor::loopingModeHorizontalPadding)
            .def_readwrite("loopingModeVerticalSquareCount", &MapDescriptor::loopingModeVerticalSquareCount)
            .def_readwrite("tourBankruptcyLimit", &MapDescriptor::tourBankruptcyLimit)
            .def_readwrite("tourInitialCash", &MapDescriptor::tourInitialCash);
}
