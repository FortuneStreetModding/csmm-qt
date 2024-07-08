#include "exewrapper.h"

#include "lib/asyncfuture/asyncfuture.h"
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

static QFuture<QString> observeProcess(QProcess *proc) {
    auto program = proc->program();
    if (proc->error() == QProcess::FailedToStart) {
        throw Exception(QString("Process '%1' failed to start").arg(program));
    }

    QObject::connect(proc, &QProcess::readyReadStandardError, [proc]() {
        qWarning() << proc->readAllStandardError();
    });

    using Args = std::tuple<int, QProcess::ExitStatus>;
    QFuture<Args> future = QtFuture::connect(proc, &QProcess::finished);

    proc->start();

    return AsyncFuture::observe(future)
        .subscribe([=](Args args) {
            const auto code = std::get<0>(args);
            const auto status = std::get<1>(args);
            if (code != 0) {
                throw Exception(QString("Process '%1' returned nonzero exit code %2").arg(program).arg(code));
            }
            QTextStream stream(proc);
            QString line = stream.readAll();
            proc->deleteLater();
            return line.trimmed();
        }).future();
}

static bool isAllDashes(const QString &line) {
    return line.trimmed().isEmpty() || line.trimmed() == QString(line.length(), '-');
}

QFuture<QVector<AddressSection>> readSections(const QString &inputFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->setProgram(getWitPath());
    proc->setArguments({"DUMP", "-l", inputFile});

    return AsyncFuture::observe(observeProcess(proc))
        .subscribe([](QString output) -> QVector<AddressSection> {
            QVector<AddressSection> result;
            QStringList lines = output.split('\n', Qt::SkipEmptyParts);
            bool startOfDeltaSection = false;
            bool startOfDeltaData = false;
            for (const QString &line : lines) {
                if (line.contains("Delta between file offset and virtual address:")) {
                    startOfDeltaSection = true;
                }
                if(startOfDeltaSection && isAllDashes(line.trimmed())) {
                    startOfDeltaData = true;
                }
                if(startOfDeltaSection && startOfDeltaData) {
                    auto columns = line.split(':');
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
                        result.append({offsetBeg, offsetEnd, fileDelta, columns[4].trimmed()});
                    }
                }
            }
            return result;
        }).future();
}

QFuture<QString> extractArcFile(const QString &arcFile, const QString &dFolder) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->setProgram(getWszstPath());
    proc->setArguments({"EXTRACT", "--overwrite", arcFile, "--dest", dFolder});
    return observeProcess(proc);
}
QFuture<QString> packDfolderToArc(const QString &dFolder, const QString &arcFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->setProgram(getWszstPath());
    proc->setArguments({"CREATE", "--overwrite", dFolder, "--dest", arcFile});
    return observeProcess(proc);
}
QFuture<QString> packTurnlotFolderToArc(const QString &dFolder, const QString &arcFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->setProgram(getWszstPath());
    proc->setArguments({"CREATE", "--overwrite", "--u8", "--no-compress", "--pt-dir=REMOVE", "--transform", "TPL.CMPR", "--n-mipmaps", "0", dFolder, "--dest", arcFile});
    return observeProcess(proc);
}
QFuture<QString> convertPngToTpl(const QString &pngFile, const QString &tplFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->setProgram(getWimgtPath());
    proc->setArguments({"ENCODE", "--overwrite", pngFile, "--dest", tplFile});
    return observeProcess(proc);
}
QFuture<QString> extractWbfsIso(const QString &wbfsFile, const QString &extractDir) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->setProgram(getWitPath());
    proc->setArguments({"COPY", "--psel", "data", "--preserve", "--overwrite", "--fst", wbfsFile, extractDir});
    return observeProcess(proc);
}
QFuture<QString> createWbfsIso(const QString &sourceDir, const QString &wbfsFile, const QString &markerCode, bool separateSaveGame) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    QString discId = separateSaveGame ? "K" : ".";
    QStringList args{"COPY", "--id", QString("%1...%2").arg(discId, markerCode), "--overwrite", sourceDir, wbfsFile};
    proc->setProgram(getWitPath());
    proc->setArguments(args);
    return observeProcess(proc);
}
QFuture<QString> patchWiimmfi(const QString &wbfsFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    QStringList args{"EDIT", "--wiimmfi", wbfsFile};
    proc->setProgram(getWitPath());
    proc->setArguments(args);
    return observeProcess(proc);
}
QFuture<QString> getId6(const QString &inputFile) {
    QProcess *proc = new QProcess();
    proc->setEnvironment(getWiimmsEnv());
    proc->setProgram((getWitPath()));
    proc->setArguments({"ID6", inputFile});
    return observeProcess(proc);
}

}
