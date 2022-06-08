#include "pythonbindings.h"
#include <pybind11/embed.h>
#include <pybind11/stl_bind.h>
#include "lib/mods/csmmmod.h"

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
static pybind11::class_<std::array<T, N>> bindStdArray(pybind11::handle h, const std::string &name, Args &&...args) {
    pybind11::class_<std::array<T, N>> cls(h, name.c_str(), std::forward<Args>(args)...);
    cls.def(pybind11::init<>());
    cls.def(pybind11::init([](const pybind11::iterable &it) {
        std::array<T, N> res;
        size_t i = 0;
        for (auto &val: it) {
            if (i >= N) throw pybind11::value_error("iterable too large");
            res[i++] = val.cast<T>();
        }
        if (i < N) throw pybind11::value_error("iterable too small");
        return res;
    }));
    pybind11::detail::vector_if_copy_constructible<std::array<T, N>, decltype(cls)>(cls);
    pybind11::detail::vector_accessor<std::array<T, N>, decltype(cls)>(cls);
    bindConstContainerOps(cls, name);
    return cls;
}

namespace {

class PyQIODevice : public QIODevice {
public:
    PyQIODevice(pybind11::object obj) : obj(obj) {}
    bool isSequential() const override {
        return !obj.attr("seekable")().cast<bool>();
    }
    bool seek(qint64 pos) override {
        obj.attr("seek")(pos);
        return true;
    }
protected:
    qint64 readData(char *data, qint64 maxlen) override {
        pybind11::bytes readData = obj.attr("read")(maxlen);
        char *dataToCopyFrom;
        pybind11::ssize_t size;
        if (PyBytes_AsStringAndSize(readData.ptr(), &dataToCopyFrom, &size) < 0) {
            throw pybind11::error_already_set();
        }
        memcpy(data, dataToCopyFrom, size);
        return size;
    }
    qint64 writeData(const char *data, qint64 len) override {
        return obj.attr("write")(pybind11::bytes(data, len)).cast<qint64>();
    }
private:
    pybind11::object obj;
};

class PyCSMMMod : public CSMMMod {
public:
    using CSMMMod::CSMMMod;

    QString modId() const override {
        PYBIND11_OVERRIDE_PURE(QString, CSMMMod, modId);
    }
    int priority() const override {
        PYBIND11_OVERRIDE(int, CSMMMod, priority);
    }
    QSet<QString> depends() const override {
        PYBIND11_OVERRIDE(QSet<QString>, CSMMMod, depends);
    }
    QSet<QString> after() const override {
        PYBIND11_OVERRIDE(QSet<QString>, CSMMMod, after);
    }
    QSet<QString> before() const override {
        PYBIND11_OVERRIDE(QSet<QString>, CSMMMod, before);
    }
};

class PyArcFileInterface : public ArcFileInterface {
private:
    typedef QMap<QString, ModifyArcFunction> ResultType;
public:
    using ArcFileInterface::ArcFileInterface;

    QMap<QString, ModifyArcFunction> modifyArcFile() override {
        PYBIND11_OVERRIDE(ResultType, ArcFileInterface, modifyArcFile);
    }
};

class PyGeneralInterface : public GeneralInterface {
public:
    using GeneralInterface::GeneralInterface;

    void loadFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) override {
        PYBIND11_OVERRIDE(void, GeneralInterface, loadFiles, root, gameInstance, modList);
    }
    void saveFiles(const QString &root, GameInstance &gameInstance, const ModListType &modList) override {
        PYBIND11_OVERRIDE(void, GeneralInterface, saveFiles, root, gameInstance, modList);
    }
};

class PyUiMessageInterface : public UiMessageInterface {
private:
    typedef QMap<QString, LoadMessagesFunction> LoadResultType;
    typedef QMap<QString, SaveMessagesFunction> SaveResultType;
public:
    using UiMessageInterface::UiMessageInterface;

    QMap<QString, LoadMessagesFunction> loadUiMessages() override {
        PYBIND11_OVERRIDE(LoadResultType, UiMessageInterface, loadUiMessages);
    }
    void allocateUiMessages(const QString &root, GameInstance &gameInstance, const ModListType &modList) override {
        PYBIND11_OVERRIDE(void, UiMessageInterface, allocateUiMessages, root, gameInstance, modList);
    }
    QMap<QString, SaveMessagesFunction> saveUiMessages() override {
        PYBIND11_OVERRIDE(SaveResultType, UiMessageInterface, saveUiMessages);
    }
};

}

PYBIND11_EMBEDDED_MODULE(pycsmm, m) {
    pybind11::enum_<RuleSet>(m, "RuleSet", R"pycsmmdoc(
    Enum representing whether the board is easy or standard mode.
)pycsmmdoc")
            .value("Easy", Easy)
            .value("Standard", Standard);

    pybind11::enum_<BoardTheme>(m, "BoardTheme", R"pycsmmdoc(
    Enum representing the board's franchise theme.
)pycsmmdoc")
            .value("DragonQuest", DragonQuest)
            .value("Mario", Mario);

    pybind11::enum_<BgmId> bgmIdEnum(m, "BgmId", R"pycsmmdoc(
    Enum representing the background music id.
)pycsmmdoc");
    auto &bgmIdMap = Bgm::bgmIdMapping();
    for (auto it=bgmIdMap.begin(); it!=bgmIdMap.end(); ++it) {
        bgmIdEnum.value(it.key().toUtf8(), it.value());
    }

    pybind11::enum_<MusicType> musicTypeEnum(m, "MusicType",  R"pycsmmdoc(
    Enum representing the usage of the music.
)pycsmmdoc");
    auto &musicTypeMap = Music::stringToMusicTypeMapping();
    for (auto it=musicTypeMap.begin(); it!=musicTypeMap.end(); ++it) {
        musicTypeEnum.value(it.key().toUtf8(), it.value());
    }

    pybind11::enum_<Character> characterEnum(m, "Character", R"pycsmmdoc(
    Enum representing the character.
)pycsmmdoc");
    auto &charactersMap = stringToTourCharactersMapping();
    for (auto it = charactersMap.begin(); it != charactersMap.end(); ++it) {
        characterEnum.value(it.key().toUtf8(), it.value());
    }

    pybind11::enum_<LoopingMode>(m, "LoopingMode", R"pycsmmdoc(
    Enum representing how the board loops.
)pycsmmdoc")
            .value("None", None)
            .value("Vertical", Vertical)
            .value("Both", Both);

    pybind11::class_<OriginPoint>(m, "OriginPoint", R"pycsmmdoc(
    An ordered pair (x,y) representing switch rotation origin point.
)pycsmmdoc")
            .def(pybind11::init([](float x, float y) { return OriginPoint{x, y}; }), pybind11::arg("x"),  pybind11::arg("y"), R"pycsmmdoc(
    Constructor.
)pycsmmdoc")
            .def_readwrite("x", &OriginPoint::x, R"pycsmmdoc(
    The x-value.
)pycsmmdoc")
            .def_readwrite("y", &OriginPoint::y, R"pycsmmdoc(
    The y-value.
)pycsmmdoc");

    pybind11::class_<MusicEntry>(m, "MusicEntry", R"pycsmmdoc(
    Represents a music entry in the music BRSAR file.
)pycsmmdoc")
            .def(pybind11::init<const QString &, quint8, qint32, quint32>(),
                 pybind11::arg("brstmBaseFilename"), pybind11::arg("volume"), pybind11::arg("brsarIndex"), pybind11::arg("brstmFileSize"), R"pycsmmdoc(
    Constructor.
)pycsmmdoc")
            .def_readwrite("brstmBaseFilename", &MusicEntry::brstmBaseFilename, R"pycsmmdoc(
    The base filename of the music file.
)pycsmmdoc")
            .def_readwrite("volume", &MusicEntry::volume, R"pycsmmdoc(
    The volume from 0 to 100.
)pycsmmdoc")
            .def_readwrite("brsarIndex", &MusicEntry::brsarIndex, R"pycsmmdoc(
    The relative location of the music entry in the BRSAR file.
)pycsmmdoc")
            .def_readwrite("brstmFileSize", &MusicEntry::brstmFileSize, R"pycsmmdoc(
    The size of the BRSTM music file.
)pycsmmdoc");

    bindStdArray<bool, 128>(m, "VentureCardTable", R"pycsmmdoc(
    A boolean array-like type such that venture card x is enabled iff VentureCardTable[x-1] is enabled.
)pycsmmdoc");

    bindStdArray<QString, 4>(m, "FrbFiles", R"pycsmmdoc(
    An array-like type containing the frb file names.
)pycsmmdoc");

    pybind11::bind_vector<std::vector<OriginPoint>>(m, "OriginPoints", R"pycsmmdoc(
    A list-like type of switch rotation origins.
)pycsmmdoc");

    pybind11::bind_map<std::map<MusicType, MusicEntry>>(m, "MusicEntryTable", R"pycsmmdoc(
    A dict-like type mapping MusicTypes to MusicEntries.
)pycsmmdoc");

    bindStdArray<Character, 3>(m, "TourCharacters", R"pycsmmdoc(
    An array-like type containing the three tour opponents.
)pycsmmdoc");

    pybind11::bind_map<std::map<QString, QString>>(m, "LanguageTable", R"pycsmmdoc(
    Maps the language code to the localized name, string, etc.
)pycsmmdoc");

    pybind11::bind_vector<std::vector<QString>>(m, "StringList", R"pycsmmdoc(
    A list-like type of strings.
)pycsmmdoc");

    pybind11::bind_map<std::map<QString, std::vector<QString>>>(m, "ListLanguageTable", R"pycsmmdoc(
    Maps the language code to a localized list of localized names, strings, etc.
)pycsmmdoc");

    pybind11::bind_vector<std::vector<quint32>>(m, "LanguageIdList", R"pycsmmdoc(
    A list-like type of localization IDs.
)pycsmmdoc");

    pybind11::bind_vector<std::vector<MapDescriptor>>(m, "MapDescriptorList", R"pycsmmdoc(
    A list-like type of map descriptors.
)pycsmmdoc");

    pybind11::bind_map<UiMessage>(m, "UiMessageMapping", R"pycsmmdoc(
    A mapping from localization ID to the localized string.
)pycsmmdoc");

    pybind11::class_<MapDescriptor>(m, "MapDescriptor", R"pycsmmdoc(
    Stores relevant information for a Fortune Street board.
)pycsmmdoc")
            .def(pybind11::init<>())
            .def_readwrite("mapSet", &MapDescriptor::mapSet, R"pycsmmdoc(
    The map set (in vanilla, 1 is standard and 0 is easy).
)pycsmmdoc")
            .def_readwrite("zone", &MapDescriptor::zone, R"pycsmmdoc(
    The map zone (0 for DQuest, 1 for Mario, 2 for Special)
)pycsmmdoc")
            .def_readwrite("order", &MapDescriptor::order, R"pycsmmdoc(
    The map order relative to other maps in the same mapSet and zone.
)pycsmmdoc")
            .def_readwrite("isPracticeBoard", &MapDescriptor::isPracticeBoard, R"pycsmmdoc(
    Whether the board is a practice board.
)pycsmmdoc")
            .def_readwrite("unlockId", &MapDescriptor::unlockId, R"pycsmmdoc(
    An ID used to determine how the board is unlocked in Tour Mode.
)pycsmmdoc")
            .def_readwrite("ruleSet", &MapDescriptor::ruleSet, R"pycsmmdoc(
    The board's rule set (Easy or Standard).
)pycsmmdoc")
            .def_readwrite("initialCash", &MapDescriptor::initialCash, R"pycsmmdoc(
    The starting ready cash.
)pycsmmdoc")
            .def_readwrite("targetAmount", &MapDescriptor::targetAmount, R"pycsmmdoc(
    The target amount.
)pycsmmdoc")
            .def_readwrite("baseSalary", &MapDescriptor::baseSalary, R"pycsmmdoc(
    The base salary.
)pycsmmdoc")
            .def_readwrite("salaryIncrement", &MapDescriptor::salaryIncrement, R"pycsmmdoc(
    The increase in salary for each level.
)pycsmmdoc")
            .def_readwrite("maxDiceRoll", &MapDescriptor::maxDiceRoll, R"pycsmmdoc(
    The maximum die roll for this board.
)pycsmmdoc")
            .def_readwrite("ventureCards", &MapDescriptor::ventureCards, R"pycsmmdoc(
    A boolean array of venture cards and whether each is enabled.
)pycsmmdoc")
            .def_readwrite("frbFiles", &MapDescriptor::frbFiles, R"pycsmmdoc(
    The Fortune Street board file names.
)pycsmmdoc")
            .def_readwrite("switchRotationOrigins", &MapDescriptor::switchRotationOrigins, R"pycsmmdoc(
    A list of points (if applicable) about which the board rotates on a switch.
)pycsmmdoc")
            .def_readwrite("theme", &MapDescriptor::theme, R"pycsmmdoc(
    The board theme (Dragon Quest/Mario).
)pycsmmdoc")
            .def_readwrite("background", &MapDescriptor::background, R"pycsmmdoc(
    The background name.
)pycsmmdoc")
            .def_readwrite("bgmId", &MapDescriptor::bgmId, R"pycsmmdoc(
    The background music ID.
)pycsmmdoc")
            .def_readwrite("mapIcon", &MapDescriptor::mapIcon, R"pycsmmdoc(
    The map icon name.
)pycsmmdoc")
            .def_readwrite("music", &MapDescriptor::music, R"pycsmmdoc(
    A mapping associating the types of music (venture, stock, etc.) to entries in the music BRSAR.
)pycsmmdoc")
            .def_readwrite("loopingMode", &MapDescriptor::loopingMode, R"pycsmmdoc(
    How the board loops, if applicable.
)pycsmmdoc")
            .def_readwrite("loopingModeRadius", &MapDescriptor::loopingModeRadius, R"pycsmmdoc(
    Radius of the cylinder that the board loops around.
)pycsmmdoc")
            .def_readwrite("loopingModeHorizontalPadding", &MapDescriptor::loopingModeHorizontalPadding, R"pycsmmdoc(
    Might have something to do with the buggy Both looping mode, probably need to ask Def about it.
)pycsmmdoc")
            .def_readwrite("loopingModeVerticalSquareCount", &MapDescriptor::loopingModeVerticalSquareCount, R"pycsmmdoc(
    The circumference of the vertical loop, in squares.
)pycsmmdoc")
            .def_readwrite("tourBankruptcyLimit", &MapDescriptor::tourBankruptcyLimit, R"pycsmmdoc(
    The bankruptcy limit for this board in Tour mode.
)pycsmmdoc")
            .def_readwrite("tourInitialCash", &MapDescriptor::tourInitialCash, R"pycsmmdoc(
    The starting ready cash in Tour mode for this board.
)pycsmmdoc")
            .def_readwrite("tourCharacters", &MapDescriptor::tourCharacters, R"pycsmmdoc(
    The opponents in Tour mode for this board.
)pycsmmdoc")
            .def_readwrite("tourClearRank", &MapDescriptor::tourClearRank, R"pycsmmdoc(
    The rank needed to clear the board on Tour mode.
)pycsmmdoc")
            .def_readwrite("nameMsgId", &MapDescriptor::nameMsgId, R"pycsmmdoc(
    The localization ID for the board name.
)pycsmmdoc")
            .def_readwrite("descMsgId", &MapDescriptor::descMsgId, R"pycsmmdoc(
    The localization ID for the board description.
)pycsmmdoc")
            .def_readwrite("names", &MapDescriptor::names, R"pycsmmdoc(
    A mapping from language code to board name.
)pycsmmdoc")
            .def_readwrite("descs", &MapDescriptor::descs, R"pycsmmdoc(
    A mapping from language code to board description.
)pycsmmdoc")
            .def_readwrite("internalName", &MapDescriptor::internalName, R"pycsmmdoc(
    The board's internal name.
)pycsmmdoc")
            .def_readwrite("mapDescriptorFilePath", &MapDescriptor::mapDescriptorFilePath, R"pycsmmdoc(
    The file path of the yaml file associated with this descriptor.
)pycsmmdoc")
            .def_readwrite("districtNames", &MapDescriptor::districtNames, R"pycsmmdoc(
    A mapping from language code to a list of district names in order.
)pycsmmdoc")
            .def_readwrite("districtNameIds", &MapDescriptor::districtNameIds, R"pycsmmdoc(
    A list of localization IDs for the boards' district names.
)pycsmmdoc")
            .def_property("extraData", [](MapDescriptor &desc) { return desc.extraData.get(); },
    [](MapDescriptor &desc, pybind11::dict d) { desc.extraData.get() = d; }, R"pycsmmdoc(
    Additional data in the map descriptor yaml, stored in the extraData key in the yaml.
)pycsmmdoc");

    pybind11::class_<AddressMapper>(m, "AddressMapper", R"pycsmmdoc(
    Class for mapping between different types of addresses in the main.dol.
)pycsmmdoc")
            .def("canConvertToFileAddress", &AddressMapper::canConvertToFileAddress, R"pycsmmdoc(
    Whether the given virtual address for this main.dol corresponds to a valid file address.
)pycsmmdoc")
            .def("toFileAddress", &AddressMapper::toFileAddress, R"pycsmmdoc(
    Converts the given virtual address for this main.dol to the corresponding file address.
)pycsmmdoc")
            .def("boomToFileAddress", &AddressMapper::boomToFileAddress, R"pycsmmdoc(
    Converts the given Boom Street virtual address to the corresponding file address.
)pycsmmdoc")
            .def("fileAddressToStandardVirtualAddress", &AddressMapper::fileAddressToStandardVirtualAddress, R"pycsmmdoc(
    Converts the given file address for this main.dol to the corresponding virtual address for this main.dol.
)pycsmmdoc")
            .def("boomStreetToStandard", &AddressMapper::boomStreetToStandard, R"pycsmmdoc(
    Converts the given Boom Street virtual address to this main.dol's corresponding virtual address.
)pycsmmdoc")
            .def("standardToBoomStreet", &AddressMapper::standardToBoomStreet, R"pycsmmdoc(
    Converts the given virtual address for this main.dol to the corresponding Boom Street virtual address.
)pycsmmdoc")
            .def("canConvertBoomStreetToStandard", &AddressMapper::canConvertBoomStreetToStandard, R"pycsmmdoc(
    Whether the given Boom Street virtual address corresponds to a valid virtual address for this main.dol.
)pycsmmdoc");

    pybind11::class_<FreeSpaceManager>(m, "FreeSpaceManager", R"pycsmmdoc(
    Class for managing the free space on the main.dol.
)pycsmmdoc")
            .def("addFreeSpace", &FreeSpaceManager::addFreeSpace, pybind11::arg("start"), pybind11::arg("end"), R"pycsmmdoc(
    Adds free space to the main.dol.
)pycsmmdoc")
            .def("calculateTotalRemainingFreeSpace", &FreeSpaceManager::calculateTotalRemainingFreeSpace, R"pycsmmdoc(
    Returns the total remaining free space left in this main.dol.
)pycsmmdoc")
            .def("calculateTotalFreeSpace", &FreeSpaceManager::calculateTotalFreeSpace, R"pycsmmdoc(
    Returns the total free space that was marked for possible allocation in this main.dol.
)pycsmmdoc")
            .def("calculateLargestFreeSpaceBlockSize", &FreeSpaceManager::calculateLargestFreeSpaceBlockSize, R"pycsmmdoc(
    Returns the largest block of free space that was marked for possible allocation in this main.dol.
)pycsmmdoc")
            .def("calculateLargestRemainingFreeSpaceBlockSize", &FreeSpaceManager::calculateLargestRemainingFreeSpaceBlockSize, R"pycsmmdoc(
    Returns the largest block of free space left in this main.dol.
)pycsmmdoc")
            .def("allocateUnusedSpace", [](FreeSpaceManager &fsm, const QByteArray &bytes, pybind11::object fileObj, const AddressMapper &fileMapper, const QString &purpose, bool reuse) {
                PyQIODevice device(fileObj);
                QDataStream stream(&device);
                return fsm.allocateUnusedSpace(bytes, stream, fileMapper, purpose, reuse);
            }, pybind11::arg("bytesToAllocate"), pybind11::arg("mainDolFileObj"), pybind11::arg("addressMapper"), pybind11::arg("purpose"), pybind11::arg("reuse") = true, R"pycsmmdoc(
    Allocates the given bytes into the main.dol.

    :return: the virtual address of the start of the allocated bytes.
)pycsmmdoc")
            .def("reset", &FreeSpaceManager::reset, R"pycsmmdoc(
    Resets the free space manager.
)pycsmmdoc");

    pybind11::class_<GameInstance>(m, "GameInstance", R"pycsmmdoc(
    Class representing information about/operations on the current Fortune Street directory/ISO.
)pycsmmdoc")
            .def_property("mapDescriptors",
                          qNonConstOverload<>(&GameInstance::mapDescriptors),
                          [](GameInstance &instance, const std::vector<MapDescriptor> &descs) { instance.mapDescriptors() = descs; }, R"pycsmmdoc(
    The list of map descriptors.
)pycsmmdoc")
            .def("addressMapper", &GameInstance::addressMapper, R"pycsmmdoc(
    Retrieves the address mapper.
)pycsmmdoc")
            .def("freeSpaceManager", qNonConstOverload<>(&GameInstance::freeSpaceManager), R"pycsmmdoc(
    Retrieves the free space manager.
)pycsmmdoc")
            .def("nextUiMessageId", &GameInstance::nextUiMessageId, R"pycsmmdoc(
    Fetches and updates the next available localization ID.
)pycsmmdoc");

    pybind11::class_<CSMMMod, PyCSMMMod, std::shared_ptr<CSMMMod>>(m, "CSMMMod", R"pycsmmdoc(
    The base class for all CSMM mods. Mods will be loaded from .py files with a variable called "mod"
    at module scope of this type.
)pycsmmdoc")
            .def(pybind11::init<>(), R"pycsmmdoc(
    Constructor. Due to how multiple inheritance works with pybind11, you'll have to invoke this constructor
    explicitly in your mod.
)pycsmmdoc")
            .def("modId", &CSMMMod::modId, R"pycsmmdoc(
    The mod id, should be the same name as the .py file.
)pycsmmdoc")
            .def("priority", &CSMMMod::priority, R"pycsmmdoc(
    The mod's priority; higher priorities load before lower priorities.
)pycsmmdoc")
            .def("depends", &CSMMMod::depends, R"pycsmmdoc(
    A set of mod ids of mods that this mod depends on. WARNING: this has no bearing on the mod loading order;
    you'll need to override the after method to control that.
)pycsmmdoc")
            .def("after", &CSMMMod::after, R"pycsmmdoc(
    A set of mod ids of mods that this mod must load after; this does NOT necessarily make said mods required.
)pycsmmdoc")
            .def("before", &CSMMMod::before, R"pycsmmdoc(
    A set of mod ids of mods that this mod must load before; this does NOT necessarily make said mods required.
)pycsmmdoc")
            .def("modpackDir", &CSMMMod::modpackDir, R"pycsmmdoc(
    The modpack directory, useful for loading config files. Will be an empty string if the default modpack is
    being used.
)pycsmmdoc");

    pybind11::class_<ArcFileInterface, PyArcFileInterface, std::shared_ptr<ArcFileInterface>>(m, "ArcFileInterface", R"pycsmmdoc(
    Mod interface for working with .arc files.
)pycsmmdoc")
            .def(pybind11::init<>(), R"pycsmmdoc(
    Constructor. Due to how multiple inheritance works with pybind11, you'll have to invoke this constructor
    explicitly in your mod.
)pycsmmdoc")
            .def("modifyArcFile", &ArcFileInterface::modifyArcFile, R"pycsmmdoc(
    Returns a mapping from .arc file (relative to the root of the Fortune Street game folder) to callback.
    Each callback should take 4 arguments: the root of the Fortune Street game folder, the GameInstance,
    the list of mods, and the directory that the .arc file was extracted to for modification. Each callback
    should modify the .arc file as it desires.
)pycsmmdoc");

    pybind11::class_<GeneralInterface, PyGeneralInterface, std::shared_ptr<GeneralInterface>>(m, "GeneralInterface", R"pycsmmdoc(
    Mod interface for general operations on the game folder (e.g. modifying main.dol, adding free space, etc.).
)pycsmmdoc")
            .def(pybind11::init<>(), R"pycsmmdoc(
    Constructor. Due to how multiple inheritance works with pybind11, you'll have to invoke this constructor
    explicitly in your mod.
)pycsmmdoc")
            .def("loadFiles", &GeneralInterface::loadFiles, R"pycsmmdoc(
    Reads the game files as applicable to the mod.
)pycsmmdoc", pybind11::arg("root"), pybind11::arg("gameInstance"), pybind11::arg("modList"))
            .def("saveFiles", &GeneralInterface::saveFiles, R"pycsmmdoc(
    Writes to the game files as applicable to the mod.
)pycsmmdoc", pybind11::arg("root"), pybind11::arg("gameInstance"), pybind11::arg("modList"));

    pybind11::class_<UiMessageInterface, PyUiMessageInterface, std::shared_ptr<UiMessageInterface>>(m, "UiMessageInterface", R"pycsmmdoc(
    Mod interface for modifying the game localization files.
)pycsmmdoc")
            .def(pybind11::init<>(), R"pycsmmdoc(
    Constructor. Due to how multiple inheritance works with pybind11, you'll have to invoke this constructor
    explicitly in your mod.
)pycsmmdoc")
            .def("loadUiMessages", &UiMessageInterface::loadUiMessages, R"pycsmmdoc(
    Returns a mapping from localization file (relative to the root of the Fortune Street game folder) to callback.
    Each callback should take 4 arguments: the root of the Fortune Street game folder, the GameInstance,
    the list of mods, and the mapping from localization ID to string that was parsed from the localization file.
    Each callback should load the localization messages as it pleases.
)pycsmmdoc")
            .def("saveUiMessages", &UiMessageInterface::saveUiMessages, R"pycsmmdoc(
    Returns a mapping from localization file (relative to the root of the Fortune Street game folder) to callback.
    Each callback should take 4 arguments: the root of the Fortune Street game folder, the GameInstance,
    the list of mods, and the mapping from localization ID to string that was parsed from the localization file.
    Each callback should save any new localization messages as it pleases by modifying the provided localization ID
    to string mapping.
)pycsmmdoc")
            .def("allocateUiMessages", &UiMessageInterface::allocateUiMessages, R"pycsmmdoc(
    Allocates any new localization IDs that this mod needs.
)pycsmmdoc", pybind11::arg("root"), pybind11::arg("gameInstance"), pybind11::arg("modList"));
}
