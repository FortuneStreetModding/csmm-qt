#include "exewrapper.h"

#include "asyncfuture.h"
extern "C" {
#include "benzin/brlan.h"
#include "benzin/brlyt.h"
}
#include <QDir>
#include <QDataStream>
#include <QProcess>
#include <QStandardPaths>

namespace ExeWrapper {

static const QString &getWitPath() {
    static QString witPath;
    if (witPath.isEmpty()) {
        witPath = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath(WIT_NAME);
    }
    return witPath;
}

static const QString &getWszstPath() {
    static QString wszstPath;
    if (wszstPath.isEmpty()) {
        wszstPath = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath(WSZST_NAME);
    }
    return wszstPath;
}

static const QString &getWimgtPath() {
    static QString wimgtPath;
    if (wimgtPath.isEmpty()) {
        wimgtPath = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath(WIMGT_NAME);
    }
    return wimgtPath;
}

static const QStringList &getWiimmsEnv() {
    static QStringList witEnv;
    if (witEnv.isEmpty()) {
        witEnv = QProcessEnvironment::systemEnvironment().toStringList();
        // mac is stupid at handling wit, so we have to do this
#ifdef Q_OS_MACOS
        witEnv.append("TERM=xterm-256color");
#endif
    }
    return witEnv;
}

static QFuture<void> observeProcess(QProcess *proc) {
    auto fut1 = AsyncFuture::observe(proc, &QProcess::errorOccurred)
            .subscribe([=](QProcess::ProcessError error) {
                auto program = proc->program();
                auto metaEnum = QMetaEnum::fromType<QProcess::ProcessError>();
                delete proc;
                throw Exception(QString("Process '%1' encountered an error: %2").arg(proc->program()).arg(metaEnum.valueToKey(error)));
            }).future();
    auto fut2 = AsyncFuture::observe(proc, QOverload<int>::of(&QProcess::finished))
            .subscribe([=](int code) {
        auto program = proc->program();
        delete proc;
        if (code != 0) {
            throw Exception(QString("Process '%1' returned nonzero exit code %2").arg(program).arg(code));
        }
    }).future();
    return (AsyncFuture::combine() << fut1 << fut2).future();
}

QFuture<QVector<AddressSection>> readSections(const QString &inputFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->start(getWitPath(), {"DUMP", "-l", inputFile});
    // have to use the old overload for now b/c AsyncFuture doesn't support
    // multiple arguments yet
    return AsyncFuture::observe(observeProcess(proc))
            .subscribe([=]() -> QVector<AddressSection> {
        QVector<AddressSection> result;
        QString line;
        QTextStream stream(proc);

        while (stream.readLineInto(&line)) {
            if (line.contains("Delta between file offset and virtual address:")) {
                stream.readLine();
                stream.readLine();
                stream.readLine();
                break;
            }
        }
        while (stream.readLineInto(&line)) {
            auto columns = line.splitRef(':');
            if (columns.size() == 5) {
                // unused = columns[0]
                auto offsets = columns[1].split("..");
                QDataStream offsetStream0(QByteArray::fromHex(offsets[0].trimmed().toLatin1()));
                QDataStream offsetStream1(QByteArray::fromHex(offsets[1].trimmed().toLatin1()));
                quint32 offsetBeg, offsetEnd;
                offsetStream0 >> offsetBeg;
                offsetStream1 >> offsetEnd;
                // size = columns[2]
                QDataStream fileDeltaStream(QByteArray::fromHex(columns[3].trimmed().toLatin1()));
                quint32 fileDelta;
                fileDeltaStream >> fileDelta;
                result.append({offsetBeg, offsetEnd, fileDelta, columns[4].trimmed().toString()});
            }
        }
        delete proc;
        return result;
    }).future();
}

QFuture<void> extractArcFile(const QString &arcFile, const QString &dFolder) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->start(getWszstPath(), {"EXTRACT", "--overwrite", arcFile, "--dest", dFolder});
    return observeProcess(proc);
}
QFuture<void> packDfolderToArc(const QString &dFolder, const QString &arcFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->start(getWszstPath(), {"CREATE", "--overwrite", dFolder, "--dest", arcFile});
    return observeProcess(proc);
}
void convertBrlytToXmlyt(const QString &brlytFile, const QString &xmlytFile) {
    auto brlytFileArr = brlytFile.toUtf8();
    auto xmlytFileArr = xmlytFile.toUtf8();
    if (QFileInfo(brlytFile).suffix() == "brlyt") {
        parse_brlyt(brlytFileArr.data(), xmlytFileArr.data());
    } else {
        parse_brlan(brlytFileArr.data(), xmlytFileArr.data());
    }
}
void convertXmlytToBrlyt(const QString &xmlytFile, const QString &brlytFile) {
    auto brlytFileArr = brlytFile.toUtf8();
    auto xmlytFileArr = xmlytFile.toUtf8();
    if (QFileInfo(brlytFile).suffix() == "brlyt") {
        make_brlyt(xmlytFileArr.data(), brlytFileArr.data());
    } else {
        make_brlan(xmlytFileArr.data(), brlytFileArr.data());
    }
}
QFuture<void> convertPngToTpl(const QString &pngFile, const QString &tplFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->start(getWimgtPath(), {"ENCODE", "--overwrite", pngFile, "--dest", tplFile});
    return observeProcess(proc);
}
QFuture<void> extractWbfsIso(const QString &wbfsFile, const QString &extractDir) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->start(getWitPath(), {"COPY", "--psel", "data", "--preserve", "--overwrite", "--fst", wbfsFile, extractDir});
    return observeProcess(proc);
}
QFuture<void> createWbfsIso(const QString &sourceDir, const QString &wbfsFile, bool patchWiimmfi) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    QStringList args{"COPY", "--id", ".....2", "--overwrite", sourceDir, wbfsFile};
    if (patchWiimmfi) args << "--wiimmfi";
    proc->start(getWitPath(), args);
    return observeProcess(proc);
}

Exception::Exception(const QString &msgVal) : message(msgVal) {}
const QString &Exception::getMessage() const { return message; }
void Exception::raise() const { throw *this; }
Exception *Exception::clone() const { return new Exception(*this); }

}
