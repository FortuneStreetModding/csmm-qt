#include "lib/python/pythonbindings.h"
#include <QString>
#include <QDir>
#include <QApplication>

static void setPyHome() {
#ifdef Q_OS_MAC
    Py_SetPythonHome(QDir(QApplication::applicationDirPath()).filePath("../Resources").toStdWString().c_str());
#else
    Py_SetPythonHome(QDir(QApplication::applicationDirPath()).filePath("py").toStdWString().c_str());
#endif
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // HACK: we need ENSUREPIP_OPTIONS set on Unix b/c pip otherwise thinks we're using a framework
    if (qEnvironmentVariableIsEmpty("ENSUREPIP_OPTIONS")) {
        qputenv("ENSUREPIP_OPTIONS", "1");
    }
    setPyHome();

    auto appArgs = app.arguments();
    QVector<std::wstring> wstrs;

    const wchar_t **args = new const wchar_t *[appArgs.length() + 1]();
    for (int i=0; i<appArgs.length(); ++i) {
        wstrs.append(appArgs[i].toStdWString());
        args[i] = wstrs.back().c_str();
    }

    int result = Py_Main(argc, const_cast<wchar_t **>(args));

    delete [] args;

    exit(result);
}
