// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * A disassembler for vector unit instruction pairs, for reading a program
 * while working on the interpreter. Output is for the terminal; listings of
 * a game's programs are game code and are not kept in the repository.
 *
 * It decodes the same instruction layouts that `vu_asm.h` encodes. A word it does not know is
 * printed as `?upper` or `?lower` with the word in hexadecimal. It is not used by the model
 * itself, only by tools.
 *
 * Sources: the instruction formats of the vector units as publicly documented.
 */

#pragma once

#include <cstdarg>
#include <cstdio>
#include <string>

#include "types.h"

namespace ps2::vudis {

/**
 * Names the fields a destination mask selects.
 *
 * @param dest The mask: bit 3 is x, bit 2 y, bit 1 z and bit 0 w (documented).
 * @return The letters of the selected fields, in the order x, y, z, w.
 */
inline std::string fields(u32 dest) {
    std::string s;

    // Bit 3 selects x.
    if (dest & 8) {
        s += 'x';
    }

    // Bit 2 selects y.
    if (dest & 4) {
        s += 'y';
    }

    // Bit 1 selects z.
    if (dest & 2) {
        s += 'z';
    }

    // Bit 0 selects w.
    if (dest & 1) {
        s += 'w';
    }

    return s;
}

/**
 * Formats a line of text like printf.
 *
 * @param format A printf format; the compiler checks it against the arguments.
 * @return The text, cut to 95 characters.
 */
inline std::string text(const char* format, ...) __attribute__((format(printf, 1, 2)));

inline std::string text(const char* format, ...) {
    // Room for one instruction's text; a longer line is cut.
    char buffer[96];
    va_list args;
    va_start(args, format);
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return buffer;
}

/**
 * Disassembles an upper instruction.
 *
 * @param code The upper instruction word.
 * @return The instruction as text, or `?upper` and the word when it is not known.
 */
inline std::string upper(u32 code) {
    static const char* const bc = "xyzw";

    // Upper word: dest bits 21-24, ft 16-20, fs 11-15, fd 6-10, opcode 0-5 (documented).
    u32 dest = (code >> 21) & 0xF, ft = (code >> 16) & 31, fs = (code >> 11) & 31,
        fd = (code >> 6) & 31, fn = code & 0x3F;
    std::string d = fields(dest);

    // The lambdas below are called in this function only, to print the common operand forms.
    // Three registers: fd, fs, ft.
    auto three = [&](const char* name) {
        return text("%s.%s vf%u, vf%u, vf%u", name, d.c_str(), fd, fs, ft);
    };

    // Three registers with a broadcast field of ft named in the opcode's low two bits.
    auto with_bc = [&](const char* name) {
        return text("%s%c.%s vf%u, vf%u, vf%u", name, bc[fn & 3], d.c_str(), fd, fs, ft);
    };

    // Two registers: fd and fs, the other operand being Q or I.
    auto with = [&](const char* name) {
        return text("%s.%s vf%u, vf%u", name, d.c_str(), fd, fs);
    };

    static const char* const group[7] = {"add", "sub", "madd", "msub", "max", "mini", "mul"};

    // Opcodes 0x00-0x1B are seven groups of four: the group is the mnemonic, the low two bits
    // the broadcast field (documented).
    if (fn < 0x1C) {
        return with_bc(group[fn >> 2]);
    }

    switch (fn) {
        case 0x1C:  // MULQ
            return with("mulq");
        case 0x1D:  // MAXI
            return with("maxi");
        case 0x1E:  // MULI
            return with("muli");
        case 0x1F:  // MINII
            return with("minii");
        case 0x20:  // ADDQ
            return with("addq");
        case 0x21:  // MADDQ
            return with("maddq");
        case 0x22:  // ADDI
            return with("addi");
        case 0x23:  // MADDI
            return with("maddi");
        case 0x24:  // SUBQ
            return with("subq");
        case 0x25:  // MSUBQ
            return with("msubq");
        case 0x26:  // SUBI
            return with("subi");
        case 0x27:  // MSUBI
            return with("msubi");
        case 0x28:  // ADD
            return three("add");
        case 0x29:  // MADD
            return three("madd");
        case 0x2A:  // MUL
            return three("mul");
        case 0x2B:  // MAX
            return three("max");
        case 0x2C:  // SUB
            return three("sub");
        case 0x2D:  // MSUB
            return three("msub");
        case 0x2E:  // OPMSUB
            return three("opmsub");
        case 0x2F:  // MINI
            return three("mini");
        default:
            // Opcodes not listed: the 0x3C-0x3F group is decoded below, the rest is unknown.
            break;
    }

    // An opcode below 0x3C that is not listed is not a known instruction.
    if (fn < 0x3C) {
        return text("?upper %08x", code);
    }

    // The special group: opcode 0x3C-0x3F, and the index is bits 6-10 above bits 0-1 (documented).
    u32 index = (((code >> 6) & 0x1F) << 2) | (code & 3);

    // The lambdas below print the forms that write the accumulator, and the conversions.
    auto acc3 = [&](const char* name) {
        return text("%s.%s acc, vf%u, vf%u", name, d.c_str(), fs, ft);
    };
    auto acc_bc = [&](const char* name) {
        return text("%s%c.%s acc, vf%u, vf%u", name, bc[index & 3], d.c_str(), fs, ft);
    };
    auto acc1 = [&](const char* name) {
        return text("%s.%s acc, vf%u", name, d.c_str(), fs);
    };
    auto convert = [&](const char* name) {
        return text("%s.%s vf%u, vf%u", name, d.c_str(), ft, fs);
    };
    static const char* const agroup[4] = {"adda", "suba", "madda", "msuba"};
    static const char* const scale[4] = {"0", "4", "12", "15"};

    // Indexes 0x00-0x0F: four groups of four, each with a broadcast field.
    if (index < 0x10) {
        return acc_bc(agroup[index >> 2]);
    }

    // Indexes 0x10-0x13: integer to float, with a fixed-point scale of 0, 4, 12 or 15 bits.
    if (index < 0x14) {
        return convert((std::string("itof") + scale[index & 3]).c_str());
    }

    // Indexes 0x14-0x17: float to integer, with the same scales.
    if (index < 0x18) {
        return convert((std::string("ftoi") + scale[index & 3]).c_str());
    }

    // Indexes 0x18-0x1B: MULA with a broadcast field.
    if (index < 0x1C) {
        return acc_bc("mula");
    }

    switch (index) {
        case 0x1C:  // MULAQ
            return acc1("mulaq");
        case 0x1D:  // ABS
            return convert("abs");
        case 0x1E:  // MULAI
            return acc1("mulai");
        case 0x1F:  // CLIP
            return text("clip vf%u, vf%u", fs, ft);
        case 0x20:  // ADDAQ
            return acc1("addaq");
        case 0x21:  // MADDAQ
            return acc1("maddaq");
        case 0x22:  // ADDAI
            return acc1("addai");
        case 0x23:  // MADDAI
            return acc1("maddai");
        case 0x24:  // SUBAQ
            return acc1("subaq");
        case 0x25:  // MSUBAQ
            return acc1("msubaq");
        case 0x26:  // SUBAI
            return acc1("subai");
        case 0x27:  // MSUBAI
            return acc1("msubai");
        case 0x28:  // ADDA
            return acc3("adda");
        case 0x29:  // MADDA
            return acc3("madda");
        case 0x2A:  // MULA
            return acc3("mula");
        case 0x2C:  // SUBA
            return acc3("suba");
        case 0x2D:  // MSUBA
            return acc3("msuba");
        case 0x2E:  // OPMULA
            return acc3("opmula");
        case 0x2F:  // NOP
            return "nop";
        default:
            // An index not listed is not a known instruction.
            return text("?upper %08x", code);
    }
}

/**
 * Disassembles a lower instruction.
 *
 * @param code The lower instruction word.
 * @param at The pair's address in instructions; a branch is shown with its target.
 * @return The instruction as text, or `?lower` and the word when it is not known.
 */
inline std::string lower(u32 code, u32 at) {
    static const char* const bc = "xyzw";

    // Lower word: opcode bits 25-31, dest 21-24, it 16-20, is 11-15, id 6-10 (documented).
    u32 op = code >> 25, dest = (code >> 21) & 0xF, it = (code >> 16) & 31, is = (code >> 11) & 31,
        id = (code >> 6) & 31;

    // The 11-bit immediate is signed: flip the sign bit and subtract it to extend it.
    s32 imm11 = static_cast<s32>(((code & 0x7FF) ^ 0x400) - 0x400);

    // The 15-bit and 12-bit immediates have their top bits in bits 21-24 and bit 21 (documented).
    u32 imm15 = ((code >> 10) & 0x7800) | (code & 0x7FF),
        imm12 = ((code >> 10) & 0x800) | (code & 0x7FF);

    // A branch counts instructions from the pair after it.
    u32 target = at + 1 + static_cast<u32>(imm11);
    std::string d = fields(dest);

    switch (op) {
        case 0x00:  // LQ
            return text("lq.%s vf%u, %d(vi%u)", d.c_str(), it, imm11, is & 15);
        case 0x01:  // SQ
            return text("sq.%s vf%u, %d(vi%u)", d.c_str(), is, imm11, it & 15);
        case 0x04:  // ILW
            return text("ilw.%s vi%u, %d(vi%u)", d.c_str(), it & 15, imm11, is & 15);
        case 0x05:  // ISW
            return text("isw.%s vi%u, %d(vi%u)", d.c_str(), it & 15, imm11, is & 15);
        case 0x08:  // IADDIU
            return text("iaddiu vi%u, vi%u, %u", it & 15, is & 15, imm15);
        case 0x09:  // ISUBIU
            return text("isubiu vi%u, vi%u, %u", it & 15, is & 15, imm15);
        case 0x10:  // FCEQ
            return text("fceq 0x%06x", code & 0xFFFFFF);
        case 0x11:  // FCSET
            return text("fcset 0x%06x", code & 0xFFFFFF);
        case 0x12:  // FCAND
            return text("fcand 0x%06x", code & 0xFFFFFF);
        case 0x13:  // FCOR
            return text("fcor 0x%06x", code & 0xFFFFFF);
        case 0x14:  // FSEQ
            return text("fseq vi%u, 0x%03x", it & 15, imm12);
        case 0x15:  // FSSET
            return text("fsset 0x%03x", imm12);
        case 0x16:  // FSAND
            return text("fsand vi%u, 0x%03x", it & 15, imm12);
        case 0x17:  // FSOR
            return text("fsor vi%u, 0x%03x", it & 15, imm12);
        case 0x18:  // FMEQ
            return text("fmeq vi%u, vi%u", it & 15, is & 15);
        case 0x1A:  // FMAND
            return text("fmand vi%u, vi%u", it & 15, is & 15);
        case 0x1B:  // FMOR
            return text("fmor vi%u, vi%u", it & 15, is & 15);
        case 0x1C:  // FCGET
            return text("fcget vi%u", it & 15);
        case 0x20:  // B
            return text("b %u", target);
        case 0x21:  // BAL
            return text("bal vi%u, %u", it & 15, target);
        case 0x24:  // JR
            return text("jr vi%u", is & 15);
        case 0x25:  // JALR
            return text("jalr vi%u, vi%u", it & 15, is & 15);
        case 0x28:  // IBEQ
            return text("ibeq vi%u, vi%u, %u", it & 15, is & 15, target);
        case 0x29:  // IBNE
            return text("ibne vi%u, vi%u, %u", it & 15, is & 15, target);
        case 0x2C:  // IBLTZ
            return text("ibltz vi%u, %u", is & 15, target);
        case 0x2D:  // IBGTZ
            return text("ibgtz vi%u, %u", is & 15, target);
        case 0x2E:  // IBLEZ
            return text("iblez vi%u, %u", is & 15, target);
        case 0x2F:  // IBGEZ
            return text("ibgez vi%u, %u", is & 15, target);
        case 0x40:  // special group
            // The special group: the function is in the low bits.
            break;

        default:
            return text("?lower %08x", code);
    }

    u32 fn = code & 0x3F;

    // Functions 0x30-0x35 are the integer arithmetic; the rest below 0x3C are unknown.
    if (fn < 0x3C) {
        switch (fn) {
            case 0x30:  // IADD
                return text("iadd vi%u, vi%u, vi%u", id & 15, is & 15, it & 15);
            case 0x31:  // ISUB
                return text("isub vi%u, vi%u, vi%u", id & 15, is & 15, it & 15);
            case 0x32:  // IADDI
                // The immediate is the 5-bit field in bits 6-10, signed.
                return text(
                    "iaddi vi%u, vi%u, %d", it & 15, is & 15, static_cast<s32>((id ^ 0x10) - 0x10)
                );
            case 0x34:  // IAND
                return text("iand vi%u, vi%u, vi%u", id & 15, is & 15, it & 15);
            case 0x35:  // IOR
                return text("ior vi%u, vi%u, vi%u", id & 15, is & 15, it & 15);
            default:
                return text("?lower %08x", code);
        }
    }

    // Field selectors: fsf in bits 21-22 and ftf in bits 23-24, where dest sits otherwise.
    u32 fsf = (code >> 21) & 3, ftf = (code >> 23) & 3;

    // The special function, as the 7-bit index of bits 6-10 above bits 0-1 (documented).
    switch ((((code >> 6) & 0x1F) << 2) | (code & 3)) {
        case 0x30:  // MOVE
            // MOVE with an empty destination and both registers 0 is the lower NOP.
            return dest == 0 && it == 0 && is == 0 ? std::string("nop")
                                                   : text("move.%s vf%u, vf%u", d.c_str(), it, is);
        case 0x31:  // MR32
            return text("mr32.%s vf%u, vf%u", d.c_str(), it, is);
        case 0x34:  // LQI
            return text("lqi.%s vf%u, (vi%u++)", d.c_str(), it, is & 15);
        case 0x35:  // SQI
            return text("sqi.%s vf%u, (vi%u++)", d.c_str(), is, it & 15);
        case 0x36:  // LQD
            return text("lqd.%s vf%u, (--vi%u)", d.c_str(), it, is & 15);
        case 0x37:  // SQD
            return text("sqd.%s vf%u, (--vi%u)", d.c_str(), is, it & 15);
        case 0x38:  // DIV
            return text("div q, vf%u%c, vf%u%c", is, bc[fsf], it, bc[ftf]);
        case 0x39:  // SQRT
            return text("sqrt q, vf%u%c", it, bc[ftf]);
        case 0x3A:  // RSQRT
            return text("rsqrt q, vf%u%c, vf%u%c", is, bc[fsf], it, bc[ftf]);
        case 0x3B:  // WAITQ
            return "waitq";
        case 0x3C:  // MTIR
            return text("mtir vi%u, vf%u%c", it & 15, is, bc[fsf]);
        case 0x3D:  // MFIR
            return text("mfir.%s vf%u, vi%u", d.c_str(), it, is & 15);
        case 0x3E:  // ILWR
            return text("ilwr.%s vi%u, (vi%u)", d.c_str(), it & 15, is & 15);
        case 0x3F:  // ISWR
            return text("iswr.%s vi%u, (vi%u)", d.c_str(), it & 15, is & 15);
        case 0x40:  // RNEXT
            return text("rnext.%s vf%u, r", d.c_str(), it);
        case 0x41:  // RGET
            return text("rget.%s vf%u, r", d.c_str(), it);
        case 0x42:  // RINIT
            return text("rinit r, vf%u%c", is, bc[fsf]);
        case 0x43:  // RXOR
            return text("rxor r, vf%u%c", is, bc[fsf]);
        case 0x64:  // MFP
            return text("mfp.%s vf%u, p", d.c_str(), it);
        case 0x68:  // XTOP
            return text("xtop vi%u", it & 15);
        case 0x69:  // XITOP
            return text("xitop vi%u", it & 15);
        case 0x6C:  // XGKICK
            return text("xgkick vi%u", is & 15);
        case 0x70:  // ESADD
            return text("esadd p, vf%u", is);
        case 0x71:  // ERSADD
            return text("ersadd p, vf%u", is);
        case 0x72:  // ELENG
            return text("eleng p, vf%u", is);
        case 0x73:  // ERLENG
            return text("erleng p, vf%u", is);
        case 0x74:  // EATANXY
            return text("eatanxy p, vf%u", is);
        case 0x75:  // EATANXZ
            return text("eatanxz p, vf%u", is);
        case 0x76:  // ESUM
            return text("esum p, vf%u", is);
        case 0x78:  // ESQRT
            return text("esqrt p, vf%u%c", is, bc[fsf]);
        case 0x79:  // ERSQRT
            return text("ersqrt p, vf%u%c", is, bc[fsf]);
        case 0x7A:  // ERCPR
            return text("ercpr p, vf%u%c", is, bc[fsf]);
        case 0x7B:  // WAITP
            return "waitp";
        case 0x7C:  // ESIN
            return text("esin p, vf%u%c", is, bc[fsf]);
        case 0x7D:  // EATAN
            return text("eatan p, vf%u%c", is, bc[fsf]);
        case 0x7E:  // EEXP
            return text("eexp p, vf%u%c", is, bc[fsf]);
        default:
            // An index not listed is not a known instruction.
            return text("?lower %08x", code);
    }
}

/**
 * One pair as a line: the address, the lower instruction (or the number for
 * I), the upper one and its end-of-program mark.
 *
 * @param at The pair's address in instructions.
 * @param upper_word The upper instruction word.
 * @param lower_word The lower instruction word, or the number for I when the upper word has the
 *     I bit.
 * @return The line, without a newline.
 */
inline std::string pair(u32 at, u32 upper_word, u32 lower_word) {
    // With the I bit (bit 31) set, the lower word is a float for I, not an instruction.
    std::string low = (upper_word & 0x80000000u)
                          ? text("loi %g", static_cast<double>(as_float(lower_word)))
                          : lower(lower_word, at);

    // The columns: address, lower half in 28 characters, upper half, [E] bit 30, [M] bit 29.
    return text(
        "%4u: %-28s | %s%s%s",
        at,
        low.c_str(),
        upper(upper_word).c_str(),
        (upper_word & 0x40000000u) ? "  [E]" : "",
        (upper_word & 0x20000000u) ? "  [M]" : ""
    );
}

}  // namespace ps2::vudis
