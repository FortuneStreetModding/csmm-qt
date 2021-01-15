#include "exewrapper.h"

#include "asyncfuture.h"
extern "C" {
#include "benzin/brlan.h"
#include "benzin/brlyt.h"
}
#include <QApplication>
#include <QDir>
#include <QDataStream>
#include <QProcess>

namespace ExeWrapper {

static const QString &getWitPath() {
    static QString witPath;
    if (witPath.isEmpty()) {
        witPath = QDir(QApplication::applicationDirPath()).filePath(WIT_NAME);
    }
    return witPath;
}

static const QString &getWszstPath() {
    static QString witPath;
    if (witPath.isEmpty()) {
        witPath = QDir(QApplication::applicationDirPath()).filePath(WSZST_NAME);
    }
    return witPath;
}

static const QString &getWimgtPath() {
    static QString witPath;
    if (witPath.isEmpty()) {
        witPath = QDir(QApplication::applicationDirPath()).filePath(WIMGT_NAME);
    }
    return witPath;
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

QFuture<QVector<AddressSection>> readSections(const QString &inputFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->start(getWitPath(), {"DUMP", "-l", inputFile});
    // have to use the old overload for now b/c AsyncFuture doesn't support
    // multiple arguments yet
    return AsyncFuture::observe(proc, QOverload<int>::of(&QProcess::finished))
            .subscribe([=](int exitCode) -> QVector<AddressSection> {
        QVector<AddressSection> result;
        if (exitCode != 0) {
            delete proc;
            // TODO report failure?
            return result;
        }
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
    proc->start(getWszstPath(), {"EXTRACT", arcFile, "--dest", dFolder});
    return AsyncFuture::observe(proc, QOverload<int>::of(&QProcess::finished))
            .subscribe([=]() { delete proc; }).future();
}
QFuture<void> packDfolderToArc(const QString &dFolder, const QString &arcFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->start(getWszstPath(), {"CREATE", "--overwrite", dFolder, "--dest", arcFile});
    return AsyncFuture::observe(proc, QOverload<int>::of(&QProcess::finished))
            .subscribe([=]() { delete proc; }).future();
}
QFuture<void> convertBryltToXmlyt(const QString &bryltFile, const QString &xmlytFile) {
    if (QFileInfo(bryltFile).suffix() == "brylt") {
        parse_brlyt(bryltFile.toUtf8().data(), xmlytFile.toUtf8().data());
    } else {
        parse_brlan(bryltFile.toUtf8().data(), xmlytFile.toUtf8().data());
    }
    auto def = AsyncFuture::deferred<void>();
    def.complete();
    return def.future();
}
QFuture<void> convertXmlytToBrylt(const QString &xmlytFile, const QString &bryltFile) {
    if (QFileInfo(bryltFile).suffix() == "brylt") {
        make_brlyt(xmlytFile.toUtf8().data(), bryltFile.toUtf8().data());
    } else {
        make_brlan(xmlytFile.toUtf8().data(), bryltFile.toUtf8().data());
    }
    auto def = AsyncFuture::deferred<void>();
    def.complete();
    return def.future();
}
QFuture<void> convertPngToTpl(const QString &pngFile, const QString &tplFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->start(getWimgtPath(), {"ENCODE", "--overwrite", pngFile, "--dest", tplFile});
    return AsyncFuture::observe(proc, QOverload<int>::of(&QProcess::finished))
            .subscribe([=]() { delete proc; }).future();
}
}
