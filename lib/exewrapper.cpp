#include "exewrapper.h"

#include "asyncfuture.h"
#include <QApplication>
#include <QDir>
#include <QDataStream>
#include <QProcess>

#ifdef Q_OS_WIN
#include <process.h>
#else
#include <unistd.h>
#endif

namespace ExeWrapper {

static QString witPath;
static QStringList witEnv;

static const QString &getWitPath() {
    if (witPath.isEmpty()) {
        witPath = QDir(QApplication::applicationDirPath()).filePath(WIT_NAME);
    }
    return witPath;
}

static const QStringList &getWitEnv() {
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
    proc->setEnvironment(getWitEnv());
    proc->start(getWitPath(), {"DUMP", "-l", inputFile});
    // have to use the old overload for now b/c AsyncFuture doesn't support
    // multiple arguments yet
    return AsyncFuture::observe(proc, QOverload<int>::of(&QProcess::finished))
            .subscribe([=](int exitCode) -> QVector<AddressSection> {
        QVector<AddressSection> result;
        if (exitCode != 0) {
            delete proc;
            // todo report failure?
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
}
