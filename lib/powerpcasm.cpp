#include "powerpcasm.h"

namespace PowerPcAsm {

quint32 make32bitValueFromPair(quint32 lisOpcode, quint32 addiOpcode) {
    // e.g. 0x3c808041 and 0x38840648

    auto upper16Bit = lisOpcode & 0x0000FFFF;
    auto lower16Bit = (qint16)(addiOpcode & 0x0000FFFF);
    if (lower16Bit < 0) {
        upper16Bit -= 1;
    }
    return (upper16Bit << 0x10) + ((quint32)lower16Bit & 0x0000FFFF);
}
Pair16Bit make16bitValuePair(quint32 addr) {
    Pair16Bit addrPair{ qint16((addr & 0xFFFF0000) >> 0x10), qint16(addr & 0x0000FFFF) };
    if (addrPair.lower < 0) {
        addrPair.upper += 1;
    }
    return addrPair;
}
qint16 getOpcodeParameter(quint32 opcode) {
    return opcode & 0x0000FFFF;
}
quint32 li(quint8 reg, qint16 value) {
    return li_opcode + ((quint32)reg << 21) + ((quint32)value & 0x0000FFFF);
}
quint32 lis(quint8 reg, qint16 value) {
    return lis_opcode + ((quint32)reg << 21) + ((quint32)value & 0x0000FFFF);
}
quint32 subi(quint8 register1, quint8 register2, qint16 value) {
    return addi(register1, register2, -value);
}
quint32 lha(quint8 register1, qint16 value, quint8 register2) {
    return lha_opcode + ((quint32)register1 << 21) + ((quint32)register2 << 16) + ((quint32)value & 0x0000FFFF);
}
quint32 addi(quint8 register1, quint8 register2, qint16 value) {
    return addi_opcode + ((quint32)register1 << 21) + ((quint32)register2 << 16) + ((quint32)value & 0x0000FFFF);
}
quint32 addis(quint8 register1, quint8 register2, qint16 value) {
    return addis_opcode + ((quint32)register1 << 21) + ((quint32)register2 << 16) + ((quint32)value & 0x0000FFFF);
}
quint32 add(quint8 register1, quint8 register2, quint8 register3) {
    return add_opcode + ((quint32)register1 << 21) + ((quint32)register2 << 16) + ((quint32)register3 << 11);
}
quint32 ori(quint8 register1, quint8 register2, qint16 value) {
    return ori_opcode + ((quint32)register1 << 21) + ((quint32)register2 << 16) + ((quint32)value & 0x0000FFFF);
}
quint32 or_(quint8 register1, quint8 register2, quint8 register3) {
    return or_opcode + ((quint32)register1 << 16) + ((quint32)register2 << 21) + ((quint32)register3 << 11);
}
quint32 mr(quint8 register1, quint8 register2) {
    return or_(register1, register2, register2);
}
quint32 cmpw(quint8 register1, quint8 register2) {
    return cmpw_opcode + ((quint32)register1 << 16) + ((quint32)register2 << 11);
}
quint32 mulli(quint8 register1, quint8 register2, qint16 value) {
    return mulli_opcode + ((quint32)register1 << 21) + ((quint32)register2 << 16) + ((quint32)value & 0x0000FFFF);
}
quint32 cmplw(quint8 register1, quint8 register2) {
    return cmplw_opcode + ((quint32)register1 << 16) + ((quint32)register2 << 11);
}
quint32 bl(quint32 startPos, quint32 targetPos) {
    quint32 offset = ((targetPos - startPos) >> 2) & 0x00FFFFFF;
    return bl_opcode + (offset << 2);
}
quint32 bl(quint32 startPos, qint32 offset, quint32 targetPos) {
    return bl(startPos + offset * 4, targetPos);
}
quint32 b(quint32 startPos, quint32 targetPos) {
    quint32 offset = ((targetPos - startPos) >> 2) & 0x00FFFFFF;
    return b_opcode + (offset << 2);
}
quint32 b(quint32 startPos, qint32 offset, quint32 targetPos) {
    return b(startPos + offset * 4, targetPos);
}
quint32 b(qint32 offset) {
    return b_opcode + ((4 * offset) & 0x0000FFFF);
}
quint32 blt(quint32 currentPos, quint32 targetPos) {
    return blt_opcode + ((targetPos - currentPos) & 0x0000FFFF);
}
quint32 blt(qint32 currentPos, qint32 targetPos) {
    return blt_opcode + ((4 * (targetPos - currentPos)) & 0x0000FFFF);
}
quint32 beq(quint32 currentPos, quint32 targetPos) {
    return beq_opcode + ((targetPos - currentPos) & 0x0000FFFF);
}
quint32 beq(qint32 currentPos, qint32 targetPos) {
    return beq(targetPos - currentPos);
}
quint32 beq(qint32 offset) {
    return beq_opcode + ((4 * offset) & 0x0000FFFF);
}
quint32 bge(qint32 currentPos, qint32 targetPos) {
    return bge(targetPos - currentPos);
}
quint32 bge(qint32 offset) {
    return bge_opcode + ((4 * offset) & 0x0000FFFF);
}
quint32 bne(qint32 currentPos, qint32 targetPos) {
    return bne(targetPos - currentPos);
}
quint32 bne(qint32 offset) {
    return bne_opcode + ((4 * offset) & 0x0000FFFF);
}
quint32 cmpwi(quint8 reg, qint16 value) {
    return cmpwi_opcode + ((quint32)reg << 16) + ((quint32)value & 0x0000FFFF);
}
quint32 cmplwi(quint8 reg, quint16 value) {
    return cmplwi_opcode + ((quint32)reg << 16) + ((quint32)value & 0x0000FFFF);
}
quint32 blr() { return blr_opcode; }
quint32 nop() { return ori(0, 0, 0); }
quint32 mflr(quint8 reg) {
    return mflr_opcode + ((quint32)reg << 21);
}
quint32 mtlr(quint8 reg) {
    return mtlr_opcode + ((quint32)reg << 21);
}
quint32 lwzx(quint8 register1, quint8 register2, quint8 register3) {
    return lwzx_opcode + ((quint32)register1 << 21) + ((quint32)register2 << 16) + ((quint32)register3 << 11);
}
quint32 lwz(quint8 register1, qint16 value, quint8 register2) {
    return lwz_opcode + ((quint32)register1 << 21) + ((quint32)register2 << 16) + ((quint32)value & 0x0000FFFF);
}
quint32 lbz(quint8 register1, qint16 value, quint8 register2) {
    return lbz_opcode + ((quint32)register1 << 21) + ((quint32)register2 << 16) + ((quint32)value & 0x0000FFFF);
}
quint32 srw(quint8 register1, quint8 register2, quint8 register3) {
    return srw_opcode + ((quint32)register1 << 16) + ((quint32)register2 << 21) + ((quint32)register3 << 11);
}
quint32 slw(quint8 register1, quint8 register2, quint8 register3) {
    return slw_opcode + ((quint32)register1 << 16) + ((quint32)register2 << 21) + ((quint32)register3 << 11);
}
quint32 rlwinm(quint8 register1, quint8 register2, quint8 value1, quint8 value2, quint8 value3) {
    return rlwinm_opcode + ((quint32)register1 << 16) + ((quint32)register2 << 21) + ((quint32)value1 << 11) + ((quint32)value2 << 6) + ((quint32)value3 << 1);
}
quint32 stw(quint8 register1, qint16 value, quint8 register2) {
    return stw_opcode + ((quint32)register1 << 21) + ((quint32)register2 << 16) + ((quint32)value & 0x0000FFFF);
}
quint32 stb(quint8 register1, qint16 value, quint8 register2) {
    return stb_opcode + ((quint32)register1 << 21) + ((quint32)register2 << 16) + ((quint32)value & 0x0000FFFF);
}
quint32 andi(quint8 register1, quint8 register2, qint16 value) {
    return andi_opcode + ((quint32)register1 << 16) + ((quint32)register2 << 21) + ((quint32)value & 0x0000FFFF);
}
quint32 stbx(quint8 register1, quint8 register2, quint8 register3) {
    return stbx_opcode + ((quint32)register1 << 21) + ((quint32)register2 << 16) + ((quint32)register3 << 11);
}
quint32 stwu(quint8 register1, qint16 value, quint8 register2) {
    return stwu_opcode + ((quint32)register1 << 21) + ((quint32)register2 << 16) + ((quint32)value & 0x0000FFFF);
}

}
