#include "exewrapper.h"

#include "asyncfuture.h"
#include "qdir.h"
#include <QApplication>
#include <QDataStream>
#include <QProcess>
#include <QStandardPaths>

namespace ExeWrapper {

static const QString &getWitPath() {
    static QString witPath;
    if (witPath.isEmpty()) {
        witPath = QDir(QApplication::applicationDirPath()).filePath("wit/bin/wit");
    }
    return witPath;
}

static const QString &getWszstPath() {
    static QString wszstPath;
    if (wszstPath.isEmpty()) {
        wszstPath = QDir(QApplication::applicationDirPath()).filePath("szs/bin/wszst");
    }
    return wszstPath;
}

static const QString &getWimgtPath() {
    static QString wimgtPath;
    if (wimgtPath.isEmpty()) {
        wimgtPath = QDir(QApplication::applicationDirPath()).filePath("szs/bin/wimgt");
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
QFuture<void> createWbfsIso(const QString &sourceDir, const QString &wbfsFile, const QString &markerCode) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    QStringList args{"COPY", "--id", QString("....%1").arg(markerCode), "--overwrite", sourceDir, wbfsFile};
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
QFuture<QString> getId6(const QString &inputFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->start(getWitPath(), {"ID6", inputFile});
    return AsyncFuture::observe(observeProcess(proc, false))
            .subscribe([=]() -> QString {
        QString line;
        QTextStream stream(proc);
        line = stream.readAll();
        delete proc;
        return line.trimmed();
    }).future();
}

}
