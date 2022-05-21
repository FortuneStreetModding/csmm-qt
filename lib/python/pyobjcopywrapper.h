#ifndef PYOBJCOPYWRAPPER_H
#define PYOBJCOPYWRAPPER_H

#include "pythonbindings.h"

template<class T>
class PyObjCopyWrapper {
public:
    PyObjCopyWrapper() = default;
    PyObjCopyWrapper(const T &val) : val(val) {}
    PyObjCopyWrapper(T &&val) : val(val) {}
    PyObjCopyWrapper(const PyObjCopyWrapper<T> &other) : val(other.copyVal()) {}
    T &get() { return val; }
    const T &get() const { return val; }
private:
    T val;
    T copyVal() const {
        auto copyModule = pybind11::module_::import("copy");
        return copyModule.attr("deepcopy")(val);
    }
};

#endif // PYOBJCOPYWRAPPER_H
