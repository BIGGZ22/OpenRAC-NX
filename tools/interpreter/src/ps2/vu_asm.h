// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * Encoders for vector unit instructions, for tests and for programs written
 * here. An instruction is a pair: `Program::add(upper, lower)` stores the
 * lower word first, as program memory does.
 *
 * Every function packs the fields of one instruction into its 32-bit word and checks nothing: a
 * register or immediate that is too wide spills into the next field. The bit layouts, the opcode
 * numbers and the meaning of each instruction are those of the vector unit's public
 * documentation, and a comment on an encoder that names an operation follows it (documented).
 *
 * Sources: the instruction formats of the vector units as publicly documented.
 */

#pragma once

#include <vector>

#include "types.h"

namespace ps2::vuasm {

/** Field masks (the `dest` of an instruction): X is bit 3 and W bit 0 (documented). */
constexpr u32 X = 8, Y = 4, Z = 2, W = 1, XY = 12, XYZ = 14, XYZW = 15;

/** Bits of the upper word: E ends the program one pair later, I makes the lower word a number. */
constexpr u32 E = 0x40000000u, I = 0x80000000u;

// --- upper ---

/**
 * Encodes an upper instruction in the common layout: a destination mask, three registers and an
 * opcode.
 *
 * @param op The opcode, bits 0-5 of the word.
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 up(u32 op, u32 dest, u32 fd, u32 fs, u32 ft) {
    // Upper word: dest bits 21-24, ft 16-20, fs 11-15, fd 6-10, opcode 0-5 (documented).
    return (dest << 21) | (ft << 16) | (fs << 11) | (fd << 6) | op;
}

/**
 * Encodes an upper instruction of the special group, whose opcode is 0x3C-0x3F plus an index.
 *
 * @param index The special opcode index: its high bits go in bits 6-10 and its low two bits in bits
 *     0-1.
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 up_special(u32 index, u32 dest, u32 fs, u32 ft) {
    // As above; opcode 0x3C-0x3F in bits 0-5, the rest of the index in bits 6-10 (documented).
    return (dest << 21) | (ft << 16) | (fs << 11) | ((index >> 2) << 6) | 0x3C | (index & 3);
}

/**
 * Encodes ADD: fd = fs + ft, in the fields of `dest`.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 add(u32 dest, u32 fd, u32 fs, u32 ft) {
    return up(0x28, dest, fd, fs, ft);
}

/**
 * Encodes MADD: fd = ACC + fs * ft, in the fields of `dest`.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 madd(u32 dest, u32 fd, u32 fs, u32 ft) {
    return up(0x29, dest, fd, fs, ft);
}

/**
 * Encodes MUL: fd = fs * ft, in the fields of `dest`.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 mul(u32 dest, u32 fd, u32 fs, u32 ft) {
    return up(0x2A, dest, fd, fs, ft);
}

/**
 * Encodes MAX: fd = the larger of fs and ft, in the fields of `dest`.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 max(u32 dest, u32 fd, u32 fs, u32 ft) {
    return up(0x2B, dest, fd, fs, ft);
}

/**
 * Encodes SUB: fd = fs - ft, in the fields of `dest`.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 sub(u32 dest, u32 fd, u32 fs, u32 ft) {
    return up(0x2C, dest, fd, fs, ft);
}

/**
 * Encodes MSUB: fd = ACC - fs * ft, in the fields of `dest`.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 msub(u32 dest, u32 fd, u32 fs, u32 ft) {
    return up(0x2D, dest, fd, fs, ft);
}

/**
 * Encodes OPMSUB: fd = ACC - the outer product term of fs and ft, the second step of a cross
 * product, in x, y and z.
 *
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 opmsub(u32 fd, u32 fs, u32 ft) {
    return up(0x2E, XYZ, fd, fs, ft);
}

/**
 * Encodes MINI: fd = the smaller of fs and ft, in the fields of `dest`.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 mini(u32 dest, u32 fd, u32 fs, u32 ft) {
    return up(0x2F, dest, fd, fs, ft);
}

/**
 * Encodes ADDbc: fd = fs + one field of ft, in the fields of `dest`. The second operand is one
 * field (`bc`: 0 x, 1 y, 2 z, 3 w) of ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @param bc The field of ft used as the second operand: 0 x, 1 y, 2 z, 3 w.
 * @return The instruction word.
 */
constexpr u32 addbc(u32 dest, u32 fd, u32 fs, u32 ft, u32 bc) {
    return up(0x00 + bc, dest, fd, fs, ft);
}

/**
 * Encodes SUBbc: fd = fs - one field of ft, in the fields of `dest`. The second operand is one
 * field (`bc`: 0 x, 1 y, 2 z, 3 w) of ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @param bc The field of ft used as the second operand: 0 x, 1 y, 2 z, 3 w.
 * @return The instruction word.
 */
constexpr u32 subbc(u32 dest, u32 fd, u32 fs, u32 ft, u32 bc) {
    return up(0x04 + bc, dest, fd, fs, ft);
}

/**
 * Encodes MADDbc: fd = ACC + fs * one field of ft, in the fields of `dest`. The second operand is
 * one field (`bc`: 0 x, 1 y, 2 z, 3 w) of ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @param bc The field of ft used as the second operand: 0 x, 1 y, 2 z, 3 w.
 * @return The instruction word.
 */
constexpr u32 maddbc(u32 dest, u32 fd, u32 fs, u32 ft, u32 bc) {
    return up(0x08 + bc, dest, fd, fs, ft);
}

/**
 * Encodes MSUBbc: fd = ACC - fs * one field of ft, in the fields of `dest`. The second operand is
 * one field (`bc`: 0 x, 1 y, 2 z, 3 w) of ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @param bc The field of ft used as the second operand: 0 x, 1 y, 2 z, 3 w.
 * @return The instruction word.
 */
constexpr u32 msubbc(u32 dest, u32 fd, u32 fs, u32 ft, u32 bc) {
    return up(0x0C + bc, dest, fd, fs, ft);
}

/**
 * Encodes MAXbc: fd = the larger of fs and one field of ft, in the fields of `dest`. The second
 * operand is one field (`bc`: 0 x, 1 y, 2 z, 3 w) of ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @param bc The field of ft used as the second operand: 0 x, 1 y, 2 z, 3 w.
 * @return The instruction word.
 */
constexpr u32 maxbc(u32 dest, u32 fd, u32 fs, u32 ft, u32 bc) {
    return up(0x10 + bc, dest, fd, fs, ft);
}

/**
 * Encodes MINIbc: fd = the smaller of fs and one field of ft, in the fields of `dest`. The second
 * operand is one field (`bc`: 0 x, 1 y, 2 z, 3 w) of ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @param bc The field of ft used as the second operand: 0 x, 1 y, 2 z, 3 w.
 * @return The instruction word.
 */
constexpr u32 minibc(u32 dest, u32 fd, u32 fs, u32 ft, u32 bc) {
    return up(0x14 + bc, dest, fd, fs, ft);
}

/**
 * Encodes MULbc: fd = fs * one field of ft, in the fields of `dest`. The second operand is one
 * field (`bc`: 0 x, 1 y, 2 z, 3 w) of ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @param bc The field of ft used as the second operand: 0 x, 1 y, 2 z, 3 w.
 * @return The instruction word.
 */
constexpr u32 mulbc(u32 dest, u32 fd, u32 fs, u32 ft, u32 bc) {
    return up(0x18 + bc, dest, fd, fs, ft);
}

/**
 * Encodes MULq: fd = fs * Q, in the fields of `dest`. The second operand is Q or I.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 mulq(u32 dest, u32 fd, u32 fs) {
    return up(0x1C, dest, fd, fs, 0);
}

/**
 * Encodes MULi: fd = fs * I, in the fields of `dest`. The second operand is Q or I.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 muli(u32 dest, u32 fd, u32 fs) {
    return up(0x1E, dest, fd, fs, 0);
}

/**
 * Encodes ADDq: fd = fs + Q, in the fields of `dest`. The second operand is Q or I.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 addq(u32 dest, u32 fd, u32 fs) {
    return up(0x20, dest, fd, fs, 0);
}

/**
 * Encodes ADDi: fd = fs + I, in the fields of `dest`. The second operand is Q or I.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 addi(u32 dest, u32 fd, u32 fs) {
    return up(0x22, dest, fd, fs, 0);
}

/**
 * Encodes SUBq: fd = fs - Q, in the fields of `dest`. The second operand is Q or I.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 subq(u32 dest, u32 fd, u32 fs) {
    return up(0x24, dest, fd, fs, 0);
}

/**
 * Encodes SUBi: fd = fs - I, in the fields of `dest`. The second operand is Q or I.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fd Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 subi(u32 dest, u32 fd, u32 fs) {
    return up(0x26, dest, fd, fs, 0);
}

/**
 * Encodes ADDA: ACC = fs + ft, in the fields of `dest`. Into the accumulator.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 adda(u32 dest, u32 fs, u32 ft) {
    return up_special(0x28, dest, fs, ft);
}

/**
 * Encodes MADDA: ACC = ACC + fs * ft, in the fields of `dest`. Into the accumulator.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 madda(u32 dest, u32 fs, u32 ft) {
    return up_special(0x29, dest, fs, ft);
}

/**
 * Encodes MULA: ACC = fs * ft, in the fields of `dest`. Into the accumulator.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 mula(u32 dest, u32 fs, u32 ft) {
    return up_special(0x2A, dest, fs, ft);
}

/**
 * Encodes SUBA: ACC = fs - ft, in the fields of `dest`. Into the accumulator.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 suba(u32 dest, u32 fs, u32 ft) {
    return up_special(0x2C, dest, fs, ft);
}

/**
 * Encodes MSUBA: ACC = ACC - fs * ft, in the fields of `dest`. Into the accumulator.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 msuba(u32 dest, u32 fs, u32 ft) {
    return up_special(0x2D, dest, fs, ft);
}

/**
 * Encodes OPMULA: ACC = the outer product term of fs and ft, the first step of a cross product,
 * in x, y and z. Into the accumulator.
 *
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 opmula(u32 fs, u32 ft) {
    return up_special(0x2E, XYZ, fs, ft);
}

/**
 * Encodes MULAbc: ACC = fs * one field of ft, in the fields of `dest`. Into the accumulator.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @param bc The field of ft used as the second operand: 0 x, 1 y, 2 z, 3 w.
 * @return The instruction word.
 */
constexpr u32 mulabc(u32 dest, u32 fs, u32 ft, u32 bc) {
    return up_special(0x18 + bc, dest, fs, ft);
}

/**
 * Encodes MADDAbc: ACC = ACC + fs * one field of ft, in the fields of `dest`. Into the accumulator.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fs Source float register, 0-31.
 * @param ft Second source float register, 0-31.
 * @param bc The field of ft used as the second operand: 0 x, 1 y, 2 z, 3 w.
 * @return The instruction word.
 */
constexpr u32 maddabc(u32 dest, u32 fs, u32 ft, u32 bc) {
    return up_special(0x08 + bc, dest, fs, ft);
}

/**
 * Encodes ITOF0: ft = the integer in fs as a float, divided by 2^0. Conversions and the rest write
 * ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param ft Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 itof0(u32 dest, u32 ft, u32 fs) {
    return up_special(0x10, dest, fs, ft);
}

/**
 * Encodes ITOF4: ft = the integer in fs as a float, divided by 2^4. Conversions and the rest write
 * ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param ft Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 itof4(u32 dest, u32 ft, u32 fs) {
    return up_special(0x11, dest, fs, ft);
}

/**
 * Encodes ITOF12: ft = the integer in fs as a float, divided by 2^12. Conversions and the rest
 * write ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param ft Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 itof12(u32 dest, u32 ft, u32 fs) {
    return up_special(0x12, dest, fs, ft);
}

/**
 * Encodes ITOF15: ft = the integer in fs as a float, divided by 2^15. Conversions and the rest
 * write ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param ft Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 itof15(u32 dest, u32 ft, u32 fs) {
    return up_special(0x13, dest, fs, ft);
}

/**
 * Encodes FTOI0: ft = fs times 2^0, as an integer. Conversions and the rest write ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param ft Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 ftoi0(u32 dest, u32 ft, u32 fs) {
    return up_special(0x14, dest, fs, ft);
}

/**
 * Encodes FTOI4: ft = fs times 2^4, as an integer. Conversions and the rest write ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param ft Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 ftoi4(u32 dest, u32 ft, u32 fs) {
    return up_special(0x15, dest, fs, ft);
}

/**
 * Encodes FTOI12: ft = fs times 2^12, as an integer. Conversions and the rest write ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param ft Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 ftoi12(u32 dest, u32 ft, u32 fs) {
    return up_special(0x16, dest, fs, ft);
}

/**
 * Encodes FTOI15: ft = fs times 2^15, as an integer. Conversions and the rest write ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param ft Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 ftoi15(u32 dest, u32 ft, u32 fs) {
    return up_special(0x17, dest, fs, ft);
}

/**
 * Encodes ABS: ft = the absolute value of fs. Conversions and the rest write ft.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param ft Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 abs(u32 dest, u32 ft, u32 fs) {
    return up_special(0x1D, dest, fs, ft);
}

/**
 * Encodes CLIP: compares the x, y and z of fs with the w of ft and adds six bits to the clipping
 * flags.
 *
 * @param fs Float register whose x, y and z are compared, 0-31.
 * @param ft Float register whose w is the bound, 0-31.
 * @return The instruction word.
 */
constexpr u32 clip(u32 fs, u32 ft) {
    return up_special(0x1F, XYZ, fs, ft);
}

/**
 * Encodes the upper NOP.
 * @return The instruction word.
 */
constexpr u32 nop() {
    return up_special(0x2F, 0, 0, 0);
}

// --- lower ---

/**
 * Encodes a lower instruction in the common layout: a destination mask, two integer registers, an
 * immediate and an opcode.
 *
 * @param op The opcode, bits 0-5 of the word.
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param it Integer or float register in bits 16-20, by the instruction.
 * @param is Register in bits 11-15, by the instruction.
 * @param imm11 Immediate; only its low 11 bits are kept.
 * @return The instruction word.
 */
constexpr u32 low(u32 op, u32 dest, u32 it, u32 is, u32 imm11) {
    // Lower word: opcode bits 25-31, dest 21-24, it 16-20, is 11-15, imm11 0-10 (documented).
    return (op << 25) | (dest << 21) | (it << 16) | (is << 11) | (imm11 & 0x7FF);
}

/**
 * Encodes a lower instruction of the special group, whose opcode is 0x40 and function 0x3C-0x3F
 * plus an index.
 *
 * @param index The special opcode index: its high bits go in bits 6-10 and its low two bits in bits
 *     0-1.
 * @param dest The destination mask field, which some instructions use for field selectors.
 * @param it Register in bits 16-20, by the instruction.
 * @param is Register in bits 11-15, by the instruction.
 * @return The instruction word.
 */
constexpr u32 low_special(u32 index, u32 dest, u32 it, u32 is) {
    // Opcode 0x40 (bits 25-31) selects the special group; the function is in bits 0-5 and 6-10.
    return (0x40u << 25) | (dest << 21) | (it << 16) | (is << 11) | ((index >> 2) << 6) | 0x3C
           | (index & 3);
}

/**
 * Encodes LQ: ft = the quadword at `is + offset` of data memory, in the fields of `dest`.
 *
 * @param dest Field mask of the fields loaded.
 * @param ft Destination float register, 0-31.
 * @param offset Signed offset in quadwords; only its low 11 bits are kept.
 * @param is Integer register holding the base address, 0-15.
 * @return The instruction word.
 */
constexpr u32 lq(u32 dest, u32 ft, s32 offset, u32 is) {
    return low(0x00, dest, ft, is, static_cast<u32>(offset));
}

/**
 * Encodes SQ: the quadword at `it + offset` of data memory = fs, in the fields of `dest`.
 *
 * @param dest Field mask of the fields stored.
 * @param fs Source float register, 0-31.
 * @param offset Signed offset in quadwords; only its low 11 bits are kept.
 * @param it Integer register holding the base address, 0-15.
 * @return The instruction word.
 */
constexpr u32 sq(u32 dest, u32 fs, s32 offset, u32 it) {
    return low(0x01, dest, it, fs, static_cast<u32>(offset));
}

/**
 * Encodes ILW: it = the 16 bits of one field of the quadword at `is + offset`.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param it Destination integer register, 0-15.
 * @param offset Signed offset in quadwords; only its low 11 bits are kept.
 * @param is Integer register holding the base address, 0-15.
 * @return The instruction word.
 */
constexpr u32 ilw(u32 dest, u32 it, s32 offset, u32 is) {
    return low(0x04, dest, it, is, static_cast<u32>(offset));
}

/**
 * Encodes ISW: one field of the quadword at `is + offset` = it.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param it Source integer register, 0-15.
 * @param offset Signed offset in quadwords; only its low 11 bits are kept.
 * @param is Integer register holding the base address, 0-15.
 * @return The instruction word.
 */
constexpr u32 isw(u32 dest, u32 it, s32 offset, u32 is) {
    return low(0x05, dest, it, is, static_cast<u32>(offset));
}

/**
 * Encodes IADDIU: it = is + imm15, unsigned.
 *
 * @param it Destination integer register, 0-15.
 * @param is Source integer register, 0-15.
 * @param imm15 Unsigned 15-bit immediate, split over bits 21-24 and 0-10.
 * @return The instruction word.
 */
constexpr u32 iaddiu(u32 it, u32 is, u32 imm15) {
    // The 15-bit immediate: top four bits in bits 21-24, the rest in bits 0-10 (documented).
    return (0x08u << 25) | ((imm15 & 0x7800) << 10) | (it << 16) | (is << 11) | (imm15 & 0x7FF);
}

/**
 * Encodes ISUBIU: it = is - imm15, unsigned.
 *
 * @param it Destination integer register, 0-15.
 * @param is Source integer register, 0-15.
 * @param imm15 Unsigned 15-bit immediate, split over bits 21-24 and 0-10.
 * @return The instruction word.
 */
constexpr u32 isubiu(u32 it, u32 is, u32 imm15) {
    // The immediate is split as for IADDIU.
    return (0x09u << 25) | ((imm15 & 0x7800) << 10) | (it << 16) | (is << 11) | (imm15 & 0x7FF);
}

/**
 * Encodes FCEQ: VI01 = whether the clipping flags equal imm24.
 *
 * @param imm24 Unsigned 24-bit value.
 * @return The instruction word.
 */
constexpr u32 fceq(u32 imm24) {
    return (0x10u << 25) | imm24;
}

/**
 * Encodes FCSET: sets the clipping flags to imm24.
 *
 * @param imm24 Unsigned 24-bit value.
 * @return The instruction word.
 */
constexpr u32 fcset(u32 imm24) {
    return (0x11u << 25) | imm24;
}

/**
 * Encodes FCAND: VI01 = the clipping flags ANDed with imm24.
 *
 * @param imm24 Unsigned 24-bit mask.
 * @return The instruction word.
 */
constexpr u32 fcand(u32 imm24) {
    return (0x12u << 25) | imm24;
}

/**
 * Encodes FCOR: VI01 = whether the clipping flags ORed with imm24 are all ones.
 *
 * @param imm24 Unsigned 24-bit mask.
 * @return The instruction word.
 */
constexpr u32 fcor(u32 imm24) {
    return (0x13u << 25) | imm24;
}

/**
 * Encodes FSAND: it = the status flags ANDed with imm12.
 *
 * @param it Destination integer register, 0-15.
 * @param imm12 Unsigned 12-bit mask.
 * @return The instruction word.
 */
constexpr u32 fsand(u32 it, u32 imm12) {
    // The 12-bit immediate has its top bit in bit 21 and the rest in bits 0-10 (documented).
    return (0x16u << 25) | ((imm12 & 0x800) << 10) | (it << 16) | (imm12 & 0x7FF);
}

/**
 * Encodes FMAND: it = the MAC flags ANDed with the integer register is.
 *
 * @param it Destination integer register, 0-15.
 * @param is Integer register holding the mask, 0-15.
 * @return The instruction word.
 */
constexpr u32 fmand(u32 it, u32 is) {
    return low(0x1A, 0, it, is, 0);
}

/**
 * Encodes FCGET: it = the clipping flags.
 *
 * @param it Destination integer register, 0-15.
 * @return The instruction word.
 */
constexpr u32 fcget(u32 it) {
    return low(0x1C, 0, it, 0, 0);
}

/**
 * Encodes B: a branch. Branch offsets count instructions from the one after the branch.
 *
 * @param offset Signed offset in instructions from the pair after the branch; only its low 11 bits
 *     are kept.
 * @return The instruction word.
 */
constexpr u32 b(s32 offset) {
    return low(0x20, 0, 0, 0, static_cast<u32>(offset));
}

/**
 * Encodes BAL: a branch that saves the return address in `it`. Branch offsets count instructions
 * from the one after the branch.
 *
 * @param it Integer register that receives the return address, 0-15.
 * @param offset Signed offset in instructions from the pair after the branch; only its low 11 bits
 *     are kept.
 * @return The instruction word.
 */
constexpr u32 bal(u32 it, s32 offset) {
    return low(0x21, 0, it, 0, static_cast<u32>(offset));
}

/**
 * Encodes JR: jumps to the address in `is`.
 *
 * @param is Integer register holding the target, 0-15.
 * @return The instruction word.
 */
constexpr u32 jr(u32 is) {
    return low(0x24, 0, 0, is, 0);
}

/**
 * Encodes JALR: jumps to the address in `is` and saves the return address in `it`.
 *
 * @param it Integer register that receives the return address, 0-15.
 * @param is Integer register holding the target, 0-15.
 * @return The instruction word.
 */
constexpr u32 jalr(u32 it, u32 is) {
    return low(0x25, 0, it, is, 0);
}

/**
 * Encodes IBEQ: a branch taken when `it` equals `is`.
 *
 * @param it First integer register, 0-15.
 * @param is Second integer register, 0-15.
 * @param offset Signed offset in instructions from the pair after the branch; only its low 11 bits
 *     are kept.
 * @return The instruction word.
 */
constexpr u32 ibeq(u32 it, u32 is, s32 offset) {
    return low(0x28, 0, it, is, static_cast<u32>(offset));
}

/**
 * Encodes IBNE: a branch taken when `it` differs from `is`.
 *
 * @param it First integer register, 0-15.
 * @param is Second integer register, 0-15.
 * @param offset Signed offset in instructions from the pair after the branch; only its low 11 bits
 *     are kept.
 * @return The instruction word.
 */
constexpr u32 ibne(u32 it, u32 is, s32 offset) {
    return low(0x29, 0, it, is, static_cast<u32>(offset));
}

/**
 * Encodes IBLTZ: a branch taken when `is` is negative.
 *
 * @param is Integer register tested, 0-15.
 * @param offset Signed offset in instructions from the pair after the branch; only its low 11 bits
 *     are kept.
 * @return The instruction word.
 */
constexpr u32 ibltz(u32 is, s32 offset) {
    return low(0x2C, 0, 0, is, static_cast<u32>(offset));
}

/**
 * Encodes IBGTZ: a branch taken when `is` is greater than zero.
 *
 * @param is Integer register tested, 0-15.
 * @param offset Signed offset in instructions from the pair after the branch; only its low 11 bits
 *     are kept.
 * @return The instruction word.
 */
constexpr u32 ibgtz(u32 is, s32 offset) {
    return low(0x2D, 0, 0, is, static_cast<u32>(offset));
}

/**
 * Encodes IBLEZ: a branch taken when `is` is zero or negative.
 *
 * @param is Integer register tested, 0-15.
 * @param offset Signed offset in instructions from the pair after the branch; only its low 11 bits
 *     are kept.
 * @return The instruction word.
 */
constexpr u32 iblez(u32 is, s32 offset) {
    return low(0x2E, 0, 0, is, static_cast<u32>(offset));
}

/**
 * Encodes IBGEZ: a branch taken when `is` is zero or positive.
 *
 * @param is Integer register tested, 0-15.
 * @param offset Signed offset in instructions from the pair after the branch; only its low 11 bits
 *     are kept.
 * @return The instruction word.
 */
constexpr u32 ibgez(u32 is, s32 offset) {
    return low(0x2F, 0, 0, is, static_cast<u32>(offset));
}

/**
 * Encodes IADD: id = is + it.
 *
 * @param id Destination integer register, 0-15.
 * @param is First source integer register, 0-15.
 * @param it Second source integer register, 0-15.
 * @return The instruction word.
 */
constexpr u32 iadd(u32 id, u32 is, u32 it) {
    return (0x40u << 25) | (it << 16) | (is << 11) | (id << 6) | 0x30;
}

/**
 * Encodes ISUB: id = is - it.
 *
 * @param id Destination integer register, 0-15.
 * @param is First source integer register, 0-15.
 * @param it Second source integer register, 0-15.
 * @return The instruction word.
 */
constexpr u32 isub(u32 id, u32 is, u32 it) {
    return (0x40u << 25) | (it << 16) | (is << 11) | (id << 6) | 0x31;
}

/**
 * Encodes IADDI: it = is + imm5.
 *
 * @param it Destination integer register, 0-15.
 * @param is Source integer register, 0-15.
 * @param imm5 Signed 5-bit immediate; only its low 5 bits are kept.
 * @return The instruction word.
 */
constexpr u32 iaddi(u32 it, u32 is, s32 imm5) {
    // The 5-bit immediate goes in bits 6-10 (documented).
    return (0x40u << 25) | (it << 16) | (is << 11) | ((static_cast<u32>(imm5) & 0x1F) << 6) | 0x32;
}

/**
 * Encodes IAND: id = is AND it.
 *
 * @param id Destination integer register, 0-15.
 * @param is First source integer register, 0-15.
 * @param it Second source integer register, 0-15.
 * @return The instruction word.
 */
constexpr u32 iand(u32 id, u32 is, u32 it) {
    return (0x40u << 25) | (it << 16) | (is << 11) | (id << 6) | 0x34;
}

/**
 * Encodes IOR: id = is OR it.
 *
 * @param id Destination integer register, 0-15.
 * @param is First source integer register, 0-15.
 * @param it Second source integer register, 0-15.
 * @return The instruction word.
 */
constexpr u32 ior(u32 id, u32 is, u32 it) {
    return (0x40u << 25) | (it << 16) | (is << 11) | (id << 6) | 0x35;
}

/**
 * Encodes MOVE: ft = fs, in the fields of `dest`.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param ft Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 move(u32 dest, u32 ft, u32 fs) {
    return low_special(0x30, dest, ft, fs);
}

/**
 * Encodes MR32: ft = fs rotated by one field (y to x, z to y, w to z, x to w), in the fields of
 * `dest`.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param ft Destination float register, 0-31.
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 mr32(u32 dest, u32 ft, u32 fs) {
    return low_special(0x31, dest, ft, fs);
}

/**
 * Encodes LQI: ft = the quadword at `is`, then `is` goes up by one.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param ft Destination float register, 0-31.
 * @param is Integer register holding the address, 0-15.
 * @return The instruction word.
 */
constexpr u32 lqi(u32 dest, u32 ft, u32 is) {
    return low_special(0x34, dest, ft, is);
}

/**
 * Encodes SQI: the quadword at `it` = fs, then `it` goes up by one.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fs Source float register, 0-31.
 * @param it Integer register holding the address, 0-15.
 * @return The instruction word.
 */
constexpr u32 sqi(u32 dest, u32 fs, u32 it) {
    return low_special(0x35, dest, it, fs);
}

/**
 * Encodes LQD: `is` goes down by one, then ft = the quadword at `is`.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param ft Destination float register, 0-31.
 * @param is Integer register holding the address, 0-15.
 * @return The instruction word.
 */
constexpr u32 lqd(u32 dest, u32 ft, u32 is) {
    return low_special(0x36, dest, ft, is);
}

/**
 * Encodes SQD: `it` goes down by one, then the quadword at `it` = fs.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param fs Source float register, 0-31.
 * @param it Integer register holding the address, 0-15.
 * @return The instruction word.
 */
constexpr u32 sqd(u32 dest, u32 fs, u32 it) {
    return low_special(0x37, dest, it, fs);
}

/**
 * Encodes DIV: Q = one field of fs divided by one field of ft. One field of each operand (`fsf`,
 * `ftf`: 0 x, 1 y, 2 z, 3 w).
 *
 * @param fs Float register of the dividend, 0-31.
 * @param fsf The field of fs used: 0 x, 1 y, 2 z, 3 w.
 * @param ft Float register of the divisor, 0-31.
 * @param ftf The field of ft used: 0 x, 1 y, 2 z, 3 w.
 * @return The instruction word.
 */
constexpr u32 div(u32 fs, u32 fsf, u32 ft, u32 ftf) {
    return low_special(0x38, (ftf << 2) | fsf, ft, fs);
}

/**
 * Encodes SQRT: Q = the square root of one field of ft.
 *
 * @param ft Float register of the radicand, 0-31.
 * @param ftf The field of ft used: 0 x, 1 y, 2 z, 3 w.
 * @return The instruction word.
 */
constexpr u32 sqrt(u32 ft, u32 ftf) {
    return low_special(0x39, ftf << 2, ft, 0);
}

/**
 * Encodes RSQRT: Q = one field of fs divided by the square root of one field of ft.
 *
 * @param fs Float register of the numerator, 0-31.
 * @param fsf The field of fs used: 0 x, 1 y, 2 z, 3 w.
 * @param ft Float register of the radicand, 0-31.
 * @param ftf The field of ft used: 0 x, 1 y, 2 z, 3 w.
 * @return The instruction word.
 */
constexpr u32 rsqrt(u32 fs, u32 fsf, u32 ft, u32 ftf) {
    return low_special(0x3A, (ftf << 2) | fsf, ft, fs);
}

/**
 * Encodes WAITQ: waits until Q is ready.
 * @return The instruction word.
 */
constexpr u32 waitq() {
    return low_special(0x3B, 0, 0, 0);
}

/**
 * Encodes MTIR: it = the low 16 bits of one field of fs.
 *
 * @param it Destination integer register, 0-15.
 * @param fs Source float register, 0-31.
 * @param fsf The field of fs read: 0 x, 1 y, 2 z, 3 w.
 * @return The instruction word.
 */
constexpr u32 mtir(u32 it, u32 fs, u32 fsf) {
    return low_special(0x3C, fsf, it, fs);
}

/**
 * Encodes MFIR: ft = is, sign extended to 32 bits, in the fields of `dest`.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param ft Destination float register, 0-31.
 * @param is Source integer register, 0-15.
 * @return The instruction word.
 */
constexpr u32 mfir(u32 dest, u32 ft, u32 is) {
    return low_special(0x3D, dest, ft, is);
}

/**
 * Encodes ILWR: it = the 16 bits of one field of the quadword at `is`.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param it Destination integer register, 0-15.
 * @param is Integer register holding the address, 0-15.
 * @return The instruction word.
 */
constexpr u32 ilwr(u32 dest, u32 it, u32 is) {
    return low_special(0x3E, dest, it, is);
}

/**
 * Encodes ISWR: one field of the quadword at `is` = it.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param it Source integer register, 0-15.
 * @param is Integer register holding the address, 0-15.
 * @return The instruction word.
 */
constexpr u32 iswr(u32 dest, u32 it, u32 is) {
    return low_special(0x3F, dest, it, is);
}

/**
 * Encodes MFP: ft = P, in the fields of `dest`.
 *
 * @param dest Field mask of the fields written: `X`, `Y`, `Z`, `W` or a sum such as `XYZW`.
 * @param ft Destination float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 mfp(u32 dest, u32 ft) {
    return low_special(0x64, dest, ft, 0);
}

/**
 * Encodes XTOP: it = the VIF's TOP register.
 *
 * @param it Destination integer register, 0-15.
 * @return The instruction word.
 */
constexpr u32 xtop(u32 it) {
    return low_special(0x68, 0, it, 0);
}

/**
 * Encodes XITOP: it = the VIF's ITOP register.
 *
 * @param it Destination integer register, 0-15.
 * @return The instruction word.
 */
constexpr u32 xitop(u32 it) {
    return low_special(0x69, 0, it, 0);
}

/**
 * Encodes XGKICK: sends the GIF packet that starts at the quadword `is` names.
 *
 * @param is Integer register holding the quadword address, 0-15.
 * @return The instruction word.
 */
constexpr u32 xgkick(u32 is) {
    return low_special(0x6C, 0, 0, is);
}

/**
 * Encodes ESADD: P = the sum of the squares of the x, y and z of fs.
 *
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 esadd(u32 fs) {
    return low_special(0x70, 0, 0, fs);
}

/**
 * Encodes ELENG: P = the length of the x, y, z vector of fs.
 *
 * @param fs Source float register, 0-31.
 * @return The instruction word.
 */
constexpr u32 eleng(u32 fs) {
    return low_special(0x72, 0, 0, fs);
}

/**
 * Encodes WAITP: waits until P is ready.
 * @return The instruction word.
 */
constexpr u32 waitp() {
    return low_special(0x7B, 0, 0, 0);
}

/**
 * Encodes the lower half of a pair that only has an upper instruction.
 * @return The instruction word.
 */
constexpr u32 lnop() {
    return move(0, 0, 0);
}

/**
 * A program being written: pairs in order, with the address of the next one.
 *
 * Not thread safe; it is a plain list of words.
 */
class Program {
public:
    /**
     * The address of the next pair to be added.
     *
     * @return The number of pairs so far, which is the address in instructions.
     */
    u32 here() const { return static_cast<u32>(words_.size() / 2); }

    /**
     * Adds a pair.
     *
     * @param upper The upper instruction word.
     * @param lower The lower instruction word; it is stored first, as program memory does.
     */
    void add(u32 upper, u32 lower) {
        words_.push_back(lower);
        words_.push_back(upper);
    }

    /**
     * Adds a pair with only a lower instruction.
     *
     * @param lower The lower instruction word; the upper half is a NOP.
     */
    void lo(u32 lower) { add(nop(), lower); }

    /**
     * Adds a pair with only an upper instruction.
     *
     * @param upper The upper instruction word; the lower half is `lnop`.
     */
    void hi(u32 upper) { add(upper, lnop()); }

    /**
     * Adds a pair whose lower word is a number for I.
     *
     * @param upper The upper instruction word; the I bit is set in it.
     * @param value The float that the lower word holds, which the pair reads as I.
     */
    void loi(u32 upper, float value) { add(upper | I, as_u32(value)); }

    /**
     * Replaces the lower word at `address` (to fill in a forward branch).
     *
     * @param address The pair's address, as `here` gave it.
     * @param lower The new lower instruction word.
     */
    void patch(u32 address, u32 lower) { words_[address * 2] = lower; }

    /** Sets the E bit on the pair before last, so the program ends with the pair after it. */
    void end() { words_[words_.size() - 3] |= E; }

    /** The program's words, lower then upper for each pair, ready to load into program memory. */
    const std::vector<u32>& words() const { return words_; }

private:
    /** The words so far, two to a pair. */
    std::vector<u32> words_;
};

}  // namespace ps2::vuasm
