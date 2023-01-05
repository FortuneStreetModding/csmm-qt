#include "lib/python/pythonbindings.h"
#include <QString>
#include <QDir>
#include <QApplication>
#include <pybind11/embed.h>

PYBIND11_EMBEDDED_MODULE(pycsmm, m) {
    init_pycsmm(m);
}

static void setPyHome(const QString &apploc) {
#ifdef Q_OS_MAC
    Py_SetPythonHome(QFileInfo(apploc).dir().filePath("../Resources").toStdWString().c_str());
#else
    Py_SetPythonHome(QFileInfo(apploc).dir().filePath("py").toStdWString().c_str());
#endif
}

#ifdef Q_OS_WIN
int wmain(int argc, wchar_t *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif
    // HACK: we need ENSUREPIP_OPTIONS set on Unix b/c pip otherwise thinks we're using a framework
    if (qEnvironmentVariableIsEmpty("ENSUREPIP_OPTIONS")) {
        qputenv("ENSUREPIP_OPTIONS", "1");
    }

#ifdef Q_OS_WIN
    setPyHome(QString::fromWCharArray(argv[0]));
#else
    setPyHome(argv[0]);
#endif

#ifdef Q_OS_WIN
    return Py_Main(argc, argv);
#else
    return Py_BytesMain(argc, argv);
#endif
}
