#ifndef PYTHONBINDINGS_H
#define PYTHONBINDINGS_H

#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <QVector>
#include "lib/mods/csmmmod_decl.h"
#include "lib/uimessage.h"

/**
 * Wrapper class for CSMM mods.
 */
class CSMMModHolder {
public:
    CSMMModHolder() = default;

    template<class T>
    static CSMMModHolder fromCppObj() {
        return CSMMModHolder(std::make_shared<T>(), pybind11::none());
    }

    static CSMMModHolder fromPyObj(pybind11::object obj) {
        return CSMMModHolder(obj.cast<std::shared_ptr<CSMMMod>>(), obj);
    }

    CSMMMod &operator*() {
        return *modPtr;
    }

    const CSMMMod &operator*() const {
        return *modPtr;
    }

    CSMMMod *operator->() {
        return modPtr.get();
    }

    const CSMMMod *operator->() const {
        return modPtr.get();
    }

    std::shared_ptr<const CSMMMod> modHandle() const {
        return modPtr;
    }

    pybind11::object pyObject() {
        return obj;
    }

    /**
     * @return the mod's interface of class T (e.g. ArcFileInterface, GeneralInterface, etc.)
     */
    template<class T>
    std::shared_ptr<T> getCapability() {
        if (!obj.is_none()) {
            try {
                return obj.cast<std::shared_ptr<T>>();
            } catch (const pybind11::cast_error &) {
                return nullptr;
            }
        }
        return std::dynamic_pointer_cast<T>(modPtr);
    }

    bool operator==(const CSMMModHolder &other) const {
        return modPtr == other.modPtr && obj.is(other.obj);
    }

    bool operator!=(const CSMMModHolder &other) const {
        return !(*this == other);
    }
private:
    CSMMModHolder(std::shared_ptr<CSMMMod> modPtr, pybind11::object obj) : modPtr(modPtr), obj(obj) {}

    std::shared_ptr<CSMMMod> modPtr;
    pybind11::object obj;
};

/**
 * @brief ModListType type for the list of mods in order
 */
typedef QVector<CSMMModHolder> ModListType;

namespace pybind11::detail {

struct csmm_mod_holder_caster {
    bool load(handle src, bool convert) {
        value = CSMMModHolder::fromPyObj(reinterpret_borrow<object>(src));
        return true;
    }

    static handle
    cast(CSMMModHolder holder, return_value_policy policy, handle parent) {
        auto pyObj = holder.pyObject();
        if (pyObj.is_none()) {
            return pybind11::cast(holder.modHandle()).release();
        }
        return pyObj.release();
    }

    PYBIND11_TYPE_CASTER(CSMMModHolder, const_name("CSMMMod"));
};
template<>
struct type_caster<CSMMModHolder> : csmm_mod_holder_caster {};

template<class T>
struct type_caster<QVector<T>> : list_caster<QVector<T>, T> {};

template<class T>
struct type_caster<QSet<T>> : set_caster<QSet<T>, T> {};

template <typename Type, typename Key, typename Value>
struct qt_map_caster {
    using key_conv = make_caster<Key>;
    using value_conv = make_caster<Value>;

    bool load(handle src, bool convert) {
        if (!isinstance<dict>(src)) {
            return false;
        }
        auto d = reinterpret_borrow<dict>(src);
        value.clear();
        for (auto it : d) {
            key_conv kconv;
            value_conv vconv;
            if (!kconv.load(it.first.ptr(), convert) || !vconv.load(it.second.ptr(), convert)) {
                return false;
            }
            value.insert(cast_op<Key &&>(std::move(kconv)), cast_op<Value &&>(std::move(vconv)));
        }
        return true;
    }

    template <typename T>
    static handle cast(T &&src, return_value_policy policy, handle parent) {
        dict d;
        return_value_policy policy_key = policy;
        return_value_policy policy_value = policy;
        if (!std::is_lvalue_reference<T>::value) {
            policy_key = return_value_policy_override<Key>::policy(policy_key);
            policy_value = return_value_policy_override<Value>::policy(policy_value);
        }
        for (auto it = src.begin(); it != src.end(); ++it) {
            auto key = reinterpret_steal<object>(
                key_conv::cast(forward_like<T>(it.key()), policy_key, parent));
            auto value = reinterpret_steal<object>(
                value_conv::cast(forward_like<T>(it.value()), policy_value, parent));
            if (!key || !value) {
                return handle();
            }
            d[key] = value;
        }
        return d.release();
    }

    PYBIND11_TYPE_CASTER(Type,
                         const_name("Dict[") + key_conv::name + const_name(", ") + value_conv::name
                             + const_name("]"));
};

template<class K, class V>
struct type_caster<QMap<K, V>> : qt_map_caster<QMap<K, V>, K, V> {};

struct qstring_caster {
    bool load(handle src, bool) {
        if (!src) {
            return false;
        }

        auto utf16Bytes = reinterpret_steal<object>(PyUnicode_AsEncodedString(src.ptr(), "utf-16", nullptr));

        if (!utf16Bytes) {
            PyErr_Clear();
            return false;
        }

        const auto *buffer  = reinterpret_cast<const QChar *>(PYBIND11_BYTES_AS_STRING(utf16Bytes.ptr()));
        size_t length = (size_t) PYBIND11_BYTES_SIZE(utf16Bytes.ptr()) / sizeof(QChar);

        // Skip BOM for UTF-16/32
        if (PYBIND11_SILENCE_MSVC_C4127(true)) {
            buffer++;
            length--;
        }

        value = QString(buffer, length);

        return true;
    }

    static handle
    cast(const QString &src, return_value_policy /* policy */, handle /* parent */) {
        const char *buffer = reinterpret_cast<const char *>(src.constData());
        ssize_t len = src.size() * sizeof(QChar);
        handle s = decode_utf16(buffer, len);
        if (!s) {
            throw error_already_set();
        }
        return s;
    }

    PYBIND11_TYPE_CASTER(QString, const_name(PYBIND11_STRING_NAME));

private:
    static handle decode_utf16(const char *buffer, ssize_t nbytes) {
#if !defined(PYPY_VERSION)
        return PyUnicode_DecodeUTF16(buffer, nbytes, nullptr, nullptr);
#else
        // PyPy segfaults when on PyUnicode_DecodeUTF16 (and possibly on PyUnicode_DecodeUTF32 as
        // well), so bypass the whole thing by just passing the encoding as a string value, which
        // works properly:
        return PyUnicode_Decode(buffer,
                                nbytes,
                                "utf-16",
                                nullptr);
#endif
    }
};

template<>
struct type_caster<QString> : qstring_caster {};

struct qbytearray_caster {
    bool load(handle src, bool) {
        if (!src) {
            return false;
        }
        if (PYBIND11_BYTES_CHECK(src.ptr())) {
            const char *bytes = PYBIND11_BYTES_AS_STRING(src.ptr());
            if (!bytes) {
                pybind11_fail("Unexpected PYBIND11_BYTES_AS_STRING() failure.");
            }
            value = QByteArray(bytes, PYBIND11_BYTES_SIZE(src.ptr()));
            return true;
        }
        if (PyByteArray_Check(src.ptr())) {
            const char *bytearray = PyByteArray_AsString(src.ptr());
            if (!bytearray) {
                pybind11_fail("Unexpected PyByteArray_AsString() failure.");
            }
            value = QByteArray(bytearray, PyByteArray_Size(src.ptr()));
            return true;
        }
        return false;
    }

    static handle cast(const QByteArray &src, return_value_policy /* policy */, handle /* parent */) {
        return bytes(src.constData(), src.size()).release();
    }

    PYBIND11_TYPE_CASTER(QByteArray, const_name(PYBIND11_BYTES_NAME));
};

}

struct OriginPoint;
enum MusicType : quint32;
enum Character : quint32;
struct MusicEntry;
struct MapDescriptor;

PYBIND11_MAKE_OPAQUE(std::array<bool, 128>);
PYBIND11_MAKE_OPAQUE(std::array<QString, 4>);
PYBIND11_MAKE_OPAQUE(std::vector<OriginPoint>);
PYBIND11_MAKE_OPAQUE(std::map<MusicType, MusicEntry>);
PYBIND11_MAKE_OPAQUE(std::array<Character, 3>);
PYBIND11_MAKE_OPAQUE(std::map<QString, QString>);
PYBIND11_MAKE_OPAQUE(std::vector<QString>);
PYBIND11_MAKE_OPAQUE(std::map<QString, std::vector<QString>>);
PYBIND11_MAKE_OPAQUE(std::vector<quint32>);
PYBIND11_MAKE_OPAQUE(std::vector<MapDescriptor>);
PYBIND11_MAKE_OPAQUE(UiMessage);

#endif // PYTHONBINDINGS_H
