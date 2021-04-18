#ifndef POWERPCASM_H
#define POWERPCASM_H

#include <QtCore>

namespace PowerPcAsm {
    static constexpr quint32 or_opcode = 0x7c000378;
    static constexpr quint32 ori_opcode = 0x60000000;
    static constexpr quint32 srw_opcode = 0x7c000430;
    static constexpr quint32 andi_opcode = 0x70000000;
    static constexpr quint32 rlwinm_opcode = 0x54000000;

    static constexpr quint32 lwzx_opcode = 0x7c00002e;
    static constexpr quint32 lwz_opcode = 0x80000000;
    static constexpr quint32 lbz_opcode = 0x88000000;
    static constexpr quint32 stbx_opcode = 0x7c0001ae;
    static constexpr quint32 stw_opcode = 0x90000000;

    static constexpr quint32 li_opcode = 0x38000000;
    static constexpr quint32 lis_opcode = 0x3c000000;
    static constexpr quint32 addi_opcode = li_opcode;
    static constexpr quint32 addis_opcode = lis_opcode;
    static constexpr quint32 mulli_opcode = 0x1c000000;
    static constexpr quint32 add_opcode = 0x7c000214;
    static constexpr quint32 lha_opcode = 0xa8000000;

    static constexpr quint32 cmpw_opcode = 0x7c000000;
    static constexpr quint32 cmplw_opcode = 0x7c000040;
    static constexpr quint32 cmpwi_opcode = 0x2c000000;
    static constexpr quint32 cmplwi_opcode = 0x28000000;

    static constexpr quint32 blt_opcode = 0x41800000;
    static constexpr quint32 blr_opcode = 0x4e800020;
    static constexpr quint32 beq_opcode = 0x41820000;
    static constexpr quint32 bne_opcode = 0x40820000;
    static constexpr quint32 bge_opcode = 0x40800000;
    static constexpr quint32 b_opcode = 0x48000000;
    static constexpr quint32 bl_opcode = 0x48000001;
    static constexpr quint32 mflr_opcode = 0x7c0802a6;
    static constexpr quint32 mtlr_opcode = 0x7c0803a6;

    struct Pair16Bit {
        qint16 upper = 0, lower = 0;
    };

    quint32 make32bitValueFromPair(quint32 lisOpcode, quint32 addiOpcode);
    Pair16Bit make16bitValuePair(quint32 addr);
    qint16 getOpcodeParameter(quint32 opcode);
    quint32 li(quint8 reg, qint16 value);
    quint32 lis(quint8 reg, qint16 value);
    quint32 subi(quint8 register1, quint8 register2, qint16 value);
    quint32 lha(quint8 register1, qint16 value, quint8 register2);
    quint32 addi(quint8 register1, quint8 register2, qint16 value);
    quint32 addis(quint8 register1, quint8 register2, qint16 value);
    quint32 add(quint8 register1, quint8 register2, quint8 register3);
    quint32 ori(quint8 register1, quint8 register2, qint16 value);
    quint32 or_(quint8 register1, quint8 register2, quint8 register3);
    quint32 mr(quint8 register1, quint8 register2);
    quint32 cmpw(quint8 register1, quint8 register2);
    quint32 mulli(quint8 register1, quint8 register2, qint16 value);
    quint32 cmplw(quint8 register1, quint8 register2);
    quint32 bl(quint32 startPos, quint32 targetPos);
    quint32 bl(quint32 startPos, qint32 offset, quint32 targetPos);
    quint32 b(quint32 startPos, quint32 targetPos);
    quint32 b(quint32 startPos, qint32 offset, quint32 targetPos);
    quint32 b(qint32 offset);
    quint32 blt(quint32 currentPos, quint32 targetPos);
    quint32 blt(qint32 currentPos, qint32 targetPos);
    quint32 beq(quint32 currentPos, qint32 offset, quint32 targetPos);
    quint32 beq(quint32 currentPos, quint32 targetPos);
    quint32 beq(qint32 currentPos, qint32 targetPos);
    quint32 beq(qint32 offset);
    quint32 bge(qint32 currentPos, qint32 targetPos);
    quint32 bge(qint32 offset);
    quint32 bne(qint32 currentPos, qint32 targetPos);
    quint32 bne(qint32 offset);
    quint32 cmpwi(quint8 reg, qint16 value);
    quint32 cmplwi(quint8 reg, quint16 value);
    quint32 blr();
    quint32 nop();
    quint32 mflr(quint8 reg);
    quint32 mtlr(quint8 reg);
    quint32 lwzx(quint8 register1, quint8 register2, quint8 register3);
    quint32 lwz(quint8 register1, qint16 value, quint8 register2);
    quint32 lbz(quint8 register1, qint16 value, quint8 register2);
    quint32 srw(quint8 register1, quint8 register2, quint8 register3);
    quint32 rlwinm(quint8 register1, quint8 register2, quint8 value1, quint8 value2, quint8 value3);
    quint32 stw(quint8 register1, qint16 value, quint8 register2);
    quint32 andi(quint8 register1, quint8 register2, qint16 value);
    quint32 stbx(quint8 register1, quint8 register2, quint8 register3);
}

#endif // POWERPCASM_H
