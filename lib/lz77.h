#ifndef LZ77_H
#define LZ77_H

#include <QDataStream>

class lz77
{
    void* _dataAddr;
    quint16* _Next;
    quint16* _First;
    quint16* _Last;
    qint32 _wIndex;
    qint32 _wLength;

    quint16 makeHash(quint8* ptr);
    qint32 findPattern(quint8* sPtr, qint32 length, qint32& matchOffset);
    void consume(quint8* ptr, qint32 length, qint32 remaining);
public:
    const qint32 WindowMask = 0xFFF;
    const qint32 WindowLength = 4096; //12 bits - 1, 1 - 4096
    qint32 PatternLength = 18;        //4 bits + 3, 3 - 18
    const qint32 MinMatch = 3;

    lz77();
    ~lz77();
    void dispose();
    qint32 compress(void* srcAddr, qint32 srcLen, QDataStream& outStream, bool extFmt);
    static void expand(void* data, void* dstAddress, qint32 dstLen, bool extFmt);
};

#endif // LZ77_H
