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

static QFuture<void> observeProcess(QProcess *proc, bool deleteProcAfterSuccess = true) {
    auto program = proc->program();
    if (proc->error() == QProcess::FailedToStart) {
        delete proc;
        auto def = AsyncFuture::deferred<void>();
        def.complete();
        return def.subscribe([=] { throw Exception(QString("Process '%1' failed to start").arg(program)); }).future();
    }
    return AsyncFuture::observe(proc, QOverload<int>::of(&QProcess::finished))
            .subscribe([=](int code) {
        if (code != 0) {
            delete proc;
            throw Exception(QString("Process '%1' returned nonzero exit code %2").arg(program).arg(code));
        }
        if (deleteProcAfterSuccess) delete proc;
    }).future();
}

QFuture<QVector<AddressSection>> readSections(const QString &inputFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->start(getWitPath(), {"DUMP", "-l", inputFile});
    // have to use the old overload for now b/c AsyncFuture doesn't support
    // multiple arguments yet
    return AsyncFuture::observe(observeProcess(proc, false))
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
QFuture<void> packTurnlotFolderToArc(const QString &dFolder, const QString &arcFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    // wszst CREATE --overwrite --u8 --no-compress --pt-dir=REMOVE --transform TPL.CMPR --n-mipmaps 0 game_turnlot_BG.d
    proc->start(getWszstPath(), {"CREATE", "--overwrite", "--u8", "--no-compress", "--pt-dir=REMOVE", "--transform", "TPL.CMPR", "--n-mipmaps", "0", dFolder, "--dest", arcFile});
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
QFuture<void> createWbfsIso(const QString &sourceDir, const QString &wbfsFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    QStringList args{"COPY", "--id", ".....2", "--overwrite", sourceDir, wbfsFile};
    proc->start(getWitPath(), args);
    return observeProcess(proc);
}
QFuture<void> patchWiimmfi(const QString &wbfsFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    QStringList args{"EDIT", "--wiimmfi", wbfsFile};
    proc->start(getWitPath(), args);
    return observeProcess(proc);
}

Exception::Exception(const QString &msgVal) : message(msgVal) {}
const QString &Exception::getMessage() const { return message; }
void Exception::raise() const { throw *this; }
Exception *Exception::clone() const { return new Exception(*this); }

}
