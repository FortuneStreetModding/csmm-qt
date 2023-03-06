#ifndef POWERPCASM_H
#define POWERPCASM_H

#include <QtCore>

namespace PowerPcAsm {
    static constexpr quint32 or_opcode = 0x7c000378;
    static constexpr quint32 ori_opcode = 0x60000000;
    static constexpr quint32 srw_opcode = 0x7c000430;
    static constexpr quint32 slw_opcode = 0x7c000030;
    static constexpr quint32 andi_opcode = 0x70000000;
    static constexpr quint32 rlwinm_opcode = 0x54000000;

    static constexpr quint32 lwzx_opcode = 0x7c00002e;
    static constexpr quint32 lwz_opcode = 0x80000000;
    static constexpr quint32 lwzu_opcode = 0x84000000;
    static constexpr quint32 lbz_opcode = 0x88000000;
    static constexpr quint32 lmw_opcode = 0xb8000000;
    static constexpr quint32 stbx_opcode = 0x7c0001ae;
    static constexpr quint32 stb_opcode = 0x98000000;
    static constexpr quint32 stw_opcode = 0x90000000;
    static constexpr quint32 stwu_opcode = 0x94000000;
    static constexpr quint32 stmw_opcode = 0xbc000000;

    static constexpr quint32 li_opcode = 0x38000000;
    static constexpr quint32 lis_opcode = 0x3c000000;
    static constexpr quint32 addi_opcode = li_opcode;
    static constexpr quint32 addis_opcode = lis_opcode;
    static constexpr quint32 mulli_opcode = 0x1c000000;
    static constexpr quint32 mullw_opcode = 0x7c0001d6;
    static constexpr quint32 divwu_opcode = 0x7c000396;
    static constexpr quint32 add_opcode = 0x7c000214;
    static constexpr quint32 subf_opcode = 0x7c000050;
    static constexpr quint32 lha_opcode = 0xa8000000;
    static constexpr quint32 lhz_opcode = 0xa0000000;

    static constexpr quint32 cmpw_opcode = 0x7c000000;
    static constexpr quint32 cmplw_opcode = 0x7c000040;
    static constexpr quint32 cmpwi_opcode = 0x2c000000;
    static constexpr quint32 cmplwi_opcode = 0x28000000;

    static constexpr quint32 blt_opcode = 0x41800000;
    static constexpr quint32 blr_opcode = 0x4e800020;
    static constexpr quint32 beq_opcode = 0x41820000;
    static constexpr quint32 ble_opcode = 0x40810000;
    static constexpr quint32 bne_opcode = 0x40820000;
    static constexpr quint32 bge_opcode = 0x40800000;
    static constexpr quint32 b_opcode = 0x48000000;
    static constexpr quint32 bl_opcode = 0x48000001;
    static constexpr quint32 mflr_opcode = 0x7c0802a6;
    static constexpr quint32 mtlr_opcode = 0x7c0803a6;

    struct Pair16Bit {
        qint16 upper = 0, lower = 0;
    };

    class PowerPcAsmException : public QException, public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
        const char *what() const noexcept override { return std::runtime_error::what(); }
        PowerPcAsmException(const QString &str) : std::runtime_error(str.toStdString()) {}
        void raise() const override { throw *this; }
        PowerPcAsmException *clone() const override { return new PowerPcAsmException(*this); }
    };

    class LabelTable {
    public:
        void define(QString label, QVector<quint32> &asmListing) {
            labelToTarget[label] = asmListing.size();
            // now that the label is defined, we can go back and fix all old opcodes
            for (const auto& elem : offsetToLabel) {
                if(elem.second == label) {
                    quint32 opcode = asmListing.at(elem.first);
                    quint32 offset = 4 * (asmListing.size() - elem.first);
                    // what kind of opcode is it?
                    if((opcode & b_opcode) == b_opcode) {
                        // its a long branch -> clear the offset
                        opcode = opcode & 0xFC000003;
                        offset = offset & 0x03FFFFFC;
                    } else {
                        // its a short branch -> clear the offset
                        opcode = opcode & 0xFFFF0003;
                        offset = offset & 0x0000FFFC;
                    }
                    asmListing.replace(elem.first, opcode | offset);
                }
            }
        }
        void checkProperlyLinked() {
            QString unlinkedLabels = "";
            for (const auto& elem : offsetToLabel) {
                if(labelToTarget.count(elem.second) == 0) {
                    unlinkedLabels.append(elem.second + " ");
                }
            }
            if(!unlinkedLabels.isEmpty()) {
                throw PowerPcAsmException(QString("The following labels are undefined: %1").arg(unlinkedLabels));
            }
        }
    private:
        std::map<int, QString> offsetToLabel;
        std::map<QString, int> labelToTarget;
        quint32 reference(QString label, const QVector<quint32> &asmListing) {
            if(labelToTarget.count(label) == 0) {
                // label does not exist yet, set new reference to label and return 0 offset
                offsetToLabel[asmListing.size()] = label;
                return 0;
            } else {
                // label already exists, calculate offset to it and return it
                return 4 * (labelToTarget[label] - asmListing.size());
            }
        }
        friend quint32 b(LabelTable &labels, QString label, const QVector<quint32> &asmListing);
        friend quint32 blt(LabelTable &labels, QString label, const QVector<quint32> &asmListing);
        friend quint32 ble(LabelTable &labels, QString label, const QVector<quint32> &asmListing);
        friend quint32 beq(LabelTable &labels, QString label, const QVector<quint32> &asmListing);
        friend quint32 bge(LabelTable &labels, QString label, const QVector<quint32> &asmListing);
        friend quint32 bne(LabelTable &labels, QString label, const QVector<quint32> &asmListing);
    };

    quint32 make32bitValueFromPair(quint32 lisOpcode, quint32 addiOpcode);
    Pair16Bit make16bitValuePair(quint32 addr);
    qint16 getOpcodeParameter(quint32 opcode);
    quint32 li(quint8 reg, qint16 value);
    quint32 lis(quint8 reg, qint16 value);
    quint32 subi(quint8 register1, quint8 register2, qint16 value);
    quint32 lha(quint8 register1, qint16 value, quint8 register2);
    quint32 lhz(quint8 register1, qint16 value, quint8 register2);
    quint32 addi(quint8 register1, quint8 register2, qint16 value);
    quint32 addis(quint8 register1, quint8 register2, qint16 value);
    quint32 add(quint8 register1, quint8 register2, quint8 register3);
    quint32 subf(quint8 register1, quint8 register2, quint8 register3);
    quint32 ori(quint8 register1, quint8 register2, qint16 value);
    quint32 or_(quint8 register1, quint8 register2, quint8 register3);
    quint32 mr(quint8 register1, quint8 register2);
    quint32 cmpw(quint8 register1, quint8 register2);
    quint32 mulli(quint8 register1, quint8 register2, qint16 value);
    quint32 mullw(quint8 register1, quint8 register2, quint8 register3);
    quint32 divwu(quint8 register1, quint8 register2, quint8 register3);
    quint32 cmplw(quint8 register1, quint8 register2);
    quint32 bl(quint32 startPos, quint32 targetPos);
    quint32 bl(quint32 startPos, int offset, quint32 targetPos);
    quint32 b(quint32 startPos, quint32 targetPos);
    quint32 b(quint32 startPos, int offset, quint32 targetPos);
    quint32 b(LabelTable &labels, QString label, const QVector<quint32> &asmListing);
    quint32 blt(int offset);
    quint32 blt(LabelTable &labels, QString label, const QVector<quint32> &asmListing);
    quint32 ble(int offset);
    quint32 ble(LabelTable &labels, QString label, const QVector<quint32> &asmListing);
    quint32 beq(int offset);
    quint32 beq(LabelTable &labels, QString label, const QVector<quint32> &asmListing);
    quint32 bge(int offset);
    quint32 bge(LabelTable &labels, QString label, const QVector<quint32> &asmListing);
    quint32 bne(int offset);
    quint32 bne(LabelTable &labels, QString label, const QVector<quint32> &asmListing);
    quint32 blr();
    quint32 cmpwi(quint8 reg, qint16 value);
    quint32 cmplwi(quint8 reg, quint16 value);
    quint32 nop();
    quint32 mflr(quint8 reg);
    quint32 mtlr(quint8 reg);
    quint32 lwzx(quint8 register1, quint8 register2, quint8 register3);
    quint32 lwz(quint8 register1, qint16 value, quint8 register2);
    quint32 lwzu(quint8 register1, qint16 value, quint8 register2);
    quint32 lbz(quint8 register1, qint16 value, quint8 register2);
    quint32 srw(quint8 register1, quint8 register2, quint8 register3);
    quint32 slw(quint8 register1, quint8 register2, quint8 register3);
    quint32 rlwinm(quint8 register1, quint8 register2, quint8 value1, quint8 value2, quint8 value3);
    quint32 stw(quint8 register1, qint16 value, quint8 register2);
    quint32 stb(quint8 register1, qint16 value, quint8 register2);
    quint32 andi(quint8 register1, quint8 register2, qint16 value);
    quint32 stbx(quint8 register1, quint8 register2, quint8 register3);
    quint32 stwu(quint8 register1, qint16 value, quint8 register2);
    quint32 stmw(quint8 register1, qint16 value, quint8 register2);
    quint32 lmw(quint8 register1, qint16 value, quint8 register2);

    QVector<quint32> backupLocalRegistersToStack(uint registerCount);
    QVector<quint32> restoreLocalRegistersFromStack(uint registerCount);
}

#endif // POWERPCASM_H
