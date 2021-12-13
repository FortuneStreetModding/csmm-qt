#include "lz77.h"
/*
lz77::lz77() {
    _dataAddr = Marshal.AllocHGlobal((0x1000 + 0x10000 + 0x10000) * 2);

    _Next = (ushort*) _dataAddr;
    _First = _Next + WindowLength;
    _Last = _First + 0x10000;
}

lz77::~lz77() {
    dispose();
}

void lz77::dispose() {
    if (_dataAddr)
    {
        Marshal.FreeHGlobal(_dataAddr);
        _dataAddr = 0;
    }

    GC.SuppressFinalize(this);
}

qint32 lz77::compress(void* srcAddr, qint32 srcLen, QDataStream& outStream, bool extFmt) {
    int dstLen = 4, bitCount;
    byte control;

    byte* sPtr = (byte*) srcAddr;
    int matchLength, matchOffset = 0;
    PatternLength = extFmt ? 0xFFFF + 0xFF + 0xF + 3 : 0xF + 3;

    //Initialize
    Memory.Fill(_First, 0x40000, 0xFF);
    _wIndex = _wLength = 0;

    //Write header
    CompressionHeader header = new CompressionHeader
    {
        Algorithm = CompressionType.LZ77,
        ExpandedSize = (uint) srcLen,
        IsExtendedLZ77 = extFmt
    };
    outStream.Write(&header, 4 + (header.LargeSize ? 4 : 0));

    List<byte> blockBuffer;
    int lastUpdate = srcLen;
    int remaining = srcLen;

    while (remaining > 0)
    {
        blockBuffer = new List<byte> {0};
        for (bitCount = 0, control = 0; bitCount < 8 && remaining > 0; bitCount++)
        {
            control <<= 1;
            if ((matchLength = findPattern(sPtr, remaining, ref matchOffset)) != 0)
            {
                int length;
                if (extFmt)
                {
                    if (matchLength >= 0xFF + 0xF + 3)
                    {
                        length = matchLength - 0xFF - 0xF - 3;
                        blockBuffer.Add((byte) (0x10 | (length >> 12)));
                        blockBuffer.Add((byte) (length >> 4));
                    }
                    else if (matchLength >= 0xF + 2)
                    {
                        length = matchLength - 0xF - 2;
                        blockBuffer.Add((byte) (length >> 4));
                    }
                    else
                    {
                        length = matchLength - 1;
                    }
                }
                else
                {
                    length = matchLength - 3;
                }

                control |= 1;
                blockBuffer.Add((byte) ((length << 4) | ((matchOffset - 1) >> 8)));
                blockBuffer.Add((byte) (matchOffset - 1));
            }
            else
            {
                matchLength = 1;
                blockBuffer.Add(*sPtr);
            }

            consume(sPtr, matchLength, remaining);
            sPtr += matchLength;
            remaining -= matchLength;
        }

        //Left-align bits
        control <<= 8 - bitCount;

        //Write buffer
        blockBuffer[0] = control;
        outStream.Write(blockBuffer.ToArray(), 0, blockBuffer.Count);
        dstLen += blockBuffer.Count;

    }

    outStream.Flush();

    return dstLen;
}

quint16 lz77::makeHash(quint8* ptr) {
    return (ushort) ((ptr[0] << 6) ^ (ptr[1] << 3) ^ ptr[2]);
}

qint32 lz77::findPattern(quint8* sPtr, qint32 length, qint32& matchOffset) {
    if (length < MinMatch)
    {
        return 0;
    }

    length = Math.Min(length, PatternLength);

    byte* mPtr;
    int bestLen = MinMatch - 1, bestOffset = 0, index;
    for (int offset = _First[makeHash(sPtr)]; offset != 0xFFFF; offset = _Next[offset])
    {
        if (offset < _wIndex)
        {
            mPtr = sPtr - _wIndex + offset;
        }
        else
        {
            mPtr = sPtr - _wLength - _wIndex + offset;
        }

        if (sPtr - mPtr < 2)
        {
            break;
        }

        for (index = bestLen + 1; --index >= 0 && mPtr[index] == sPtr[index];)
        {
            ;
        }

        if (index >= 0)
        {
            continue;
        }

        for (index = bestLen; ++index < length && mPtr[index] == sPtr[index];)
        {
            ;
        }

        bestOffset = (int) (sPtr - mPtr);
        if ((bestLen = index) == length)
        {
            break;
        }
    }

    if (bestLen < MinMatch)
    {
        return 0;
    }

    matchOffset = bestOffset;
    return bestLen;
}

void lz77::consume(quint8* ptr, qint32 length, qint32 remaining) {
    int last, inOffset, inVal, outVal;
    for (int i = Math.Min(length, remaining - 2); i-- > 0;)
    {
        if (_wLength == WindowLength)
        {
            //Remove node
            outVal = makeHash(ptr - WindowLength);
            if ((_First[outVal] = _Next[_First[outVal]]) == 0xFFFF)
            {
                _Last[outVal] = 0xFFFF;
            }

            inOffset = _wIndex++;
            _wIndex &= WindowMask;
        }
        else
        {
            inOffset = _wLength++;
        }

        inVal = makeHash(ptr++);
        if ((last = _Last[inVal]) == 0xFFFF)
        {
            _First[inVal] = (ushort) inOffset;
        }
        else
        {
            _Next[last] = (ushort) inOffset;
        }

        _Last[inVal] = (ushort) inOffset;
        _Next[inOffset] = 0xFFFF;
    }
}

void lz77::expand(void* data, void* dstAddress, qint32 dstLen, bool extFmt) {
    for (byte* srcPtr = (byte*) data, dstPtr = (byte*) dstAddress, ceiling = dstPtr + dstLen; dstPtr < ceiling;)
    {
        for (byte control = *srcPtr++, bit = 8; bit-- != 0 && dstPtr != ceiling;)
        {
            if ((control & (1 << bit)) == 0)
            {
                *dstPtr++ = *srcPtr++;
            }
            else
            {
                int temp = *srcPtr >> 4,
                    num = !extFmt
                        ? temp + 3
                        : temp == 1
                            ? (((*srcPtr++ & 0x0F) << 12) | (*srcPtr++ << 4) | (*srcPtr >> 4)) + 0xFF + 0xF + 3
                            : temp == 0
                                ? (((*srcPtr++ & 0x0F) << 4) | (*srcPtr >> 4)) + 0xF + 2
                                : temp + 1,
                    offset = (((*srcPtr++ & 0xF) << 8) | *srcPtr++) + 2;
                while (dstPtr != ceiling && num-- > 0)
                {
                    *dstPtr++ = dstPtr[-offset];
                }
            }
        }
    }
}
*/
