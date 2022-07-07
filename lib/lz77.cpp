#include "lz77.h"
#include "libsais/libsais.h"
#include <QtEndian>
#include <set>

namespace LZ77 {

#define CHECK_DEST_SZ result.second.size() < result.first.size

std::pair<Archive, QByteArray> extract(QDataStream &src) {
    std::pair<Archive, QByteArray> result;
    // for some weird reason cmpres uses little endian
    src.setByteOrder(QDataStream::LittleEndian);
    quint32 algoAndSize;
    src >> algoAndSize;
    result.first.algorithm = algoAndSize & 0xF;
    result.first.size = algoAndSize >> 8;
    if ((result.first.algorithm & 0xF) != LZ77) {
        throw Exception("algorithm is not LZ77");
    }
    result.first.isExtended = (algoAndSize & 0xFF) >> 4;
    if (result.first.size == 0) {
        quint32 largeSize;
        src >> largeSize;
        result.first.size = largeSize;
    }
    src.setByteOrder(QDataStream::BigEndian);
    while (CHECK_DEST_SZ) {
        quint8 control; src >> control;
        for (int bit = 8-1; bit >= 0 && CHECK_DEST_SZ; --bit) {
            quint8 initVal;
            src >> initVal;
            if ((control & (1U << bit)) == 0) {
                result.second.push_back(initVal);
            } else {
                quint8 temp = initVal >> 4;
                quint8 remInitVal = initVal;
                quint32 num;
                if (!result.first.isExtended) { // non-extended format supports runs of at most 0xF + 3
                    num = quint32(temp) + 3;
                } else if (temp == 1) { // <= 0xFFFF + 0xFF + 0xF + 3
                    quint8 val1, val2;
                    src >> val1 >> val2;
                    num = (((quint32(initVal) & 0xF) << 12)
                           | (quint32(val1) << 4)
                           | (quint32(val2) >> 4)) + 0xFF + 0xF + 3;
                    remInitVal = val2;
                } else if (temp == 0) { // <= 0xFF + 0xF + 2
                    quint8 val1;
                    src >> val1;
                    num = (((quint32(initVal) & 0xF) << 4)
                           | (quint32(val1) >> 4)) + 0xF + 2;
                    remInitVal = val1;
                } else { // <= 0xF + 1
                    num = quint32(temp) + 1;
                }
                quint8 termOffsetVal;
                src >> termOffsetVal;
                quint32 offset = (((quint32(remInitVal) & 0xF) << 8) | termOffsetVal) + 1;
                for (quint32 i=0; i<num && CHECK_DEST_SZ; ++i) {
                    result.second.push_back(result.second[result.second.size() - offset]);
                }
            }
        }
    }
    return result;
}

static constexpr int SLIDING_WINDOW_SZ = 4096;
static constexpr int MIN_MATCH = 3;

static inline qint32 matchLen(const QByteArray &src, qint32 candIdx, qint32 curIdx, qint32 MAX_RUN_SIZE) {
    qint32 i = 0;
    for ( ; i < MAX_RUN_SIZE && curIdx + i < src.size(); ++i) {
        if (src[candIdx + i] != src[curIdx + i]) {
            break;
        }
    }
    return i;
}

void compress(const QByteArray &src, QDataStream &dest, bool isExtended)
{
    const qint32 MAX_RUN_SIZE = isExtended ? 0xFFFF + 0xFF + 0xF + 3 : 0xF + 3;

    quint32 sz = src.size();

    dest.setByteOrder(QDataStream::LittleEndian);
    quint32 header = (LZ77 << 0) | (isExtended << 4) | ((sz > 0xFFFFFF ? 0 : sz) << 8);
    dest << header;
    if (sz > 0xFFFFFF) { // large size?
        dest << sz;
    }
    dest.setByteOrder(QDataStream::BigEndian);

    auto suffixArray = std::make_unique<qint32[]>(src.size());
    // compute the suffix array (suffix order -> starting suffix index)
    libsais((const uint8_t *)src.constData(), suffixArray.get(), src.size(), 0, nullptr);

    // starting suffix index -> suffix order
    auto invSuffixArray = std::make_unique<qint32[]>(src.size());
    for (qint32 i=0; i<src.size(); ++i) {
        invSuffixArray[suffixArray[i]] = i;
    }

    // window sorted by suffix order
    auto suffixCmp = [&](quint32 A, quint32 B) {
        return invSuffixArray[A] < invSuffixArray[B];
    };
    std::set<qint32, decltype(suffixCmp)> window(suffixCmp);

    quint8 curControl = 0;
    QByteArray controlBuffer;
    QDataStream controlStream(&controlBuffer, QIODevice::WriteOnly);

    for (qint32 pos=0, controlIdx=0; pos<src.size(); controlIdx = (controlIdx+1) % 8) {
        // add pos to window for comparison
        auto it = window.insert(pos).first;
        qint32 runningSuffix = -1;
        qint32 runningMatchLen = 0;
        // check previous element as candidate
        if (it != window.begin()) {
            auto candSuffix = *std::prev(it);
            auto candMatchLen = matchLen(src, candSuffix, pos, MAX_RUN_SIZE);
            if (runningMatchLen < candMatchLen) {
                runningSuffix = candSuffix;
                runningMatchLen = candMatchLen;
            }
        }
        // check next element as candidate
        if (std::next(it) != window.end()) {
            auto candSuffix = *std::next(it);
            auto candMatchLen = matchLen(src, candSuffix, pos, MAX_RUN_SIZE);
            if (runningMatchLen < candMatchLen) {
                runningSuffix = candSuffix;
                runningMatchLen = candMatchLen;
            }
        }
        // update buffer
        qint32 newPos = pos;
        if (runningMatchLen >= MIN_MATCH) { // if match length is above threshold then compress
            curControl |= (0x1U << (8 - 1 - controlIdx));
            quint32 offset = pos - runningSuffix;
            if (!isExtended) {
                controlStream << quint16( ((runningMatchLen - 3) << 12) | (offset - 1) );
            } else if (runningMatchLen <= 0xF + 1) {
                controlStream << quint16( ((runningMatchLen - 1) << 12) | (offset - 1) );
            } else if (runningMatchLen <= 0xFF + 0xF + 2) {
                quint32 toWrite24Bit = (quint32(runningMatchLen - 0xF - 2) << 12) | (offset - 1);
                controlStream << quint8(toWrite24Bit >> 16) << quint8(toWrite24Bit >> 8)
                              << quint8(toWrite24Bit);
            } else {
                controlStream << (UINT32_C(0x10000000)
                                  | (quint32(runningMatchLen - 0xFF - 0xF - 3) << 12)
                                  | (offset - 1));
            }
            newPos += runningMatchLen;
        } else { // otherwise write byte as is
            controlStream << quint8(src[pos]);
            ++newPos;
        }
        if (controlIdx == 8 - 1) { // write control byte and data
            dest << curControl;
            dest.writeRawData(controlBuffer.constData(), controlBuffer.size());
            curControl = 0;
            controlStream.device()->seek(0);
            controlBuffer.clear();
        }
        if (pos >= SLIDING_WINDOW_SZ) {
            window.erase(pos - SLIDING_WINDOW_SZ);
        }
        for (qint32 i = pos + 1; i < newPos; ++i) {
            window.insert(i);
            if (i >= SLIDING_WINDOW_SZ) {
                window.erase(i - SLIDING_WINDOW_SZ);
            }
        }
        pos = newPos;
    }

    if (!controlBuffer.isEmpty()) { // write remaining control byte/data
        dest << curControl;
        dest.writeRawData(controlBuffer.constData(), controlBuffer.size());
    }
}

}
