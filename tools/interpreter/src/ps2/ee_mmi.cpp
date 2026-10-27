// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The EE's multimedia instructions: operations on the 128-bit registers as vectors of bytes,
 * halfwords, words or doublewords, and the second multiply and divide unit (HI1, LO1).
 *
 * This file implements the slice of `Ee` (declared in `ee.h`) that runs opcode 0x1C and its four
 * sub-tables MMI0 to MMI3.
 *
 * Sources: the Emotion Engine's multimedia instructions as publicly documented.
 */

#include <algorithm>
#include <limits>

#include "ee.h"

namespace ps2 {
namespace {

/** The 128-bit general register type of the EE. */
using Reg = Ee::Reg;

/**
 * Reads one element of a register.
 *
 * @tparam T The element type; its size sets how many elements the register holds.
 * @param r The register.
 * @param n The element number, counted from the low end.
 * @return The element.
 */
template <typename T>
inline T get(const Reg& r, unsigned n) {
    // Element 0 is the low end because the host is little-endian and the bytes are read in order.
    return load<T>(reinterpret_cast<const u8*>(&r) + n * sizeof(T));
}

/**
 * Writes one element of a register.
 *
 * @tparam T The element type; its size sets how many elements the register holds.
 * @param r The register to change.
 * @param n The element number, counted from the low end.
 * @param v The new value of the element.
 */
template <typename T>
inline void put(Reg& r, unsigned n, T v) {
    // The same byte layout as `get`.
    store<T>(reinterpret_cast<u8*>(&r) + n * sizeof(T), v);
}

/**
 * Applies one operation to every element of two registers.
 *
 * @tparam T The element type.
 * @tparam F The operation: called with an element of `a` and the matching element of `b`.
 * @param a The first register.
 * @param b The second register.
 * @param f The operation.
 * @return A register of the results.
 */
template <typename T, typename F>
inline Reg each(const Reg& a, const Reg& b, F f) {
    Reg out;

    // A register is 16 bytes, so it holds 16 / sizeof(T) elements.
    for (unsigned n = 0; n < 16 / sizeof(T); n++) {
        put<T>(out, n, static_cast<T>(f(get<T>(a, n), get<T>(b, n))));
    }

    return out;
}

/**
 * Limits a wide value to the range of a narrower type.
 *
 * @tparam T The narrow type, whose smallest and largest values are the limits.
 * @tparam Wide A type that holds every value that can occur.
 * @param v The value.
 * @return The value, or the nearest limit when it is outside the range of `T`.
 */
template <typename T, typename Wide>
inline T saturate(Wide v) {
    // The comparison is made in the wide type, so the limits do not wrap; the result then fits.
    return static_cast<T>(
        std::clamp<Wide>(v, std::numeric_limits<T>::min(), std::numeric_limits<T>::max())
    );
}

/**
 * Sign-extends a 32-bit word to 64 bits, as the EE does when it keeps a word result.
 *
 * @param v The word.
 * @return The word's value as a signed 64-bit number, as bits.
 */
inline u64 sext32(u32 v) {
    return static_cast<u64>(static_cast<s64>(static_cast<s32>(v)));
}

/**
 * Interleaves the lower (or upper) halves of two registers, t's element first.
 *
 * @tparam T The element type.
 * @param s The register that gives the odd elements of the result.
 * @param t The register that gives the even elements of the result.
 * @param upper True to take the upper half of each register, false for the lower half.
 * @return The interleaved register.
 */
template <typename T>
inline Reg interleave(const Reg& s, const Reg& t, bool upper) {
    Reg out;

    // A half register is 8 bytes: this many elements, and the upper half starts after them.
    unsigned count = 8 / sizeof(T), base = upper ? count : 0;

    // Each pair of result elements is one element of t, then the one of s at the same place.
    for (unsigned n = 0; n < count; n++) {
        put<T>(out, n * 2, get<T>(t, base + n));
        put<T>(out, n * 2 + 1, get<T>(s, base + n));
    }

    return out;
}

/**
 * Keeps the even elements of t, then of s.
 *
 * @tparam T The element type.
 * @param s The register whose even elements fill the upper half of the result.
 * @param t The register whose even elements fill the lower half of the result.
 * @return The packed register.
 */
template <typename T>
inline Reg pack(const Reg& s, const Reg& t) {
    Reg out;

    // A half register is 8 bytes, which is this many elements.
    unsigned count = 8 / sizeof(T);

    // Element n of each half comes from element 2n of the source.
    for (unsigned n = 0; n < count; n++) {
        put<T>(out, n, get<T>(t, n * 2));
        put<T>(out, count + n, get<T>(s, n * 2));
    }

    return out;
}

/**
 * Builds a register from elements of t picked by a table.
 *
 * @tparam T The element type.
 * @tparam N The number of elements in the result, and in the table.
 * @param t The source register.
 * @param order For each result element, the number of the element of `t` that goes there.
 * @return The shuffled register.
 */
template <typename T, unsigned N>
inline Reg shuffle(const Reg& t, const unsigned (&order)[N]) {
    Reg out;

    // Result element n is element order[n] of t.
    for (unsigned n = 0; n < N; n++) {
        put<T>(out, n, get<T>(t, order[n]));
    }

    return out;
}

/** The two results of a 32-bit division. */
struct Division {
    /** The quotient and the remainder, as the words LO and HI receive them. */
    u32 quotient, remainder;
};

/**
 * Divides two signed words, with results for the cases C++ leaves undefined.
 *
 * A zero divisor gives a quotient of 1 for a negative numerator and -1 for any other, and the
 * numerator as the remainder. The most negative number divided by -1 gives itself with a remainder
 * of 0 (assumed).
 *
 * @param n The numerator.
 * @param d The divisor.
 * @return The quotient and the remainder.
 */
inline Division divide(s32 n, s32 d) {
    // A zero divisor does not trap; it gives the fixed results described above.
    if (d == 0) {
        return {n < 0 ? 1u : 0xFFFFFFFFu, static_cast<u32>(n)};
    }

    // The one quotient that does not fit in 32 bits, which C++ leaves undefined.
    if (n == INT32_MIN && d == -1) {
        return {0x80000000u, 0};
    }

    return {static_cast<u32>(n / d), static_cast<u32>(n % d)};
}

/**
 * Divides two unsigned words, with results for a zero divisor.
 *
 * A zero divisor gives a quotient of all ones and the numerator as the remainder (assumed).
 *
 * @param n The numerator.
 * @param d The divisor.
 * @return The quotient and the remainder.
 */
inline Division divide_unsigned(u32 n, u32 d) {
    // A zero divisor does not trap; it gives the fixed results described above.
    if (d == 0) {
        return {0xFFFFFFFFu, n};
    }

    return {n / d, n % d};
}

}  // namespace

void Ee::mmi(u32 op) {
    // Instruction fields: rs bits 21-25, rt 16-20, rd 11-15, shift amount 6-10 (documented).
    unsigned rs = (op >> 21) & 31, rt = (op >> 16) & 31, rd = (op >> 11) & 31,
             shift = (op >> 6) & 31;

    // The low words of rs and rt, and rt as a whole register.
    u32 s = static_cast<u32>(gpr[rs].lo), t = static_cast<u32>(gpr[rt].lo);
    const Reg T = gpr[rt];

    // The vector result, and whether it goes to rd after the switch.
    Reg out;
    bool write = false;

    // The function field, bits 0-5, chooses the instruction (documented).
    switch (op & 0x3F) {
        case 0x00: {  // MADD
            // HI:LO is one 64-bit value; the sum goes back into it and rd gets LO too (documented).
            s64 sum = static_cast<s64>((hi << 32) | (lo & 0xFFFFFFFFu))
                      + static_cast<s64>(static_cast<s32>(s)) * static_cast<s32>(t);
            lo = sext32(static_cast<u32>(sum));
            hi = sext32(static_cast<u32>(sum >> 32));
            set64(rd, lo);
            break;
        }

        case 0x01: {  // MADDU
            // The same as MADD with unsigned operands (documented).
            u64 sum = ((hi << 32) | (lo & 0xFFFFFFFFu)) + static_cast<u64>(s) * t;
            lo = sext32(static_cast<u32>(sum));
            hi = sext32(static_cast<u32>(sum >> 32));
            set64(rd, lo);
            break;
        }

        case 0x04: {  // PLZCW
            // Counts the leading bits equal to the sign bit, less one, in each low word.
            auto count = [](u32 v) -> u32 {
                u32 n = 0;
                u32 sign = v >> 31;

                // Bit 31 is the sign itself, so the walk starts at bit 30 and ends at a bit that
                // differs.
                for (int bit = 30; bit >= 0 && ((v >> bit) & 1) == sign; bit--) {
                    n++;
                }

                return n;
            };

            // Each count is a 32-bit result; the second one belongs in the upper word.
            set64(
                rd, count(s) | (static_cast<u64>(count(static_cast<u32>(gpr[rs].lo >> 32))) << 32)
            );
            break;
        }

        case 0x08:  // MMI0
            mmi0(op);
            break;

        case 0x09:  // MMI2
            mmi2(op);
            break;

        case 0x10:  // MFHI1
            set64(rd, hi1);
            break;

        case 0x11:  // MTHI1
            hi1 = gpr[rs].lo;
            break;

        case 0x12:  // MFLO1
            set64(rd, lo1);
            break;

        case 0x13:  // MTLO1
            lo1 = gpr[rs].lo;
            break;

        case 0x18: {  // MULT1
            // The second unit's MULT: the product goes to HI1:LO1, and rd also gets LO1.
            s64 product = static_cast<s64>(static_cast<s32>(s)) * static_cast<s32>(t);
            lo1 = sext32(static_cast<u32>(product));
            hi1 = sext32(static_cast<u32>(product >> 32));
            set64(rd, lo1);
            break;
        }

        case 0x19: {  // MULTU1
            // As MULT1 with unsigned operands.
            u64 product = static_cast<u64>(s) * t;
            lo1 = sext32(static_cast<u32>(product));
            hi1 = sext32(static_cast<u32>(product >> 32));
            set64(rd, lo1);
            break;
        }

        case 0x1A: {  // DIV1
            // The quotient goes to LO1 and the remainder to HI1; rd is not written.
            Division d = divide(static_cast<s32>(s), static_cast<s32>(t));
            lo1 = sext32(d.quotient);
            hi1 = sext32(d.remainder);
            break;
        }

        case 0x1B: {  // DIVU1
            // As DIV1 with unsigned operands.
            Division d = divide_unsigned(s, t);
            lo1 = sext32(d.quotient);
            hi1 = sext32(d.remainder);
            break;
        }

        case 0x20: {  // MADD1
            // MADD on the second unit.
            s64 sum = static_cast<s64>((hi1 << 32) | (lo1 & 0xFFFFFFFFu))
                      + static_cast<s64>(static_cast<s32>(s)) * static_cast<s32>(t);
            lo1 = sext32(static_cast<u32>(sum));
            hi1 = sext32(static_cast<u32>(sum >> 32));
            set64(rd, lo1);
            break;
        }

        case 0x21: {  // MADDU1
            // As MADD1 with unsigned operands.
            u64 sum = ((hi1 << 32) | (lo1 & 0xFFFFFFFFu)) + static_cast<u64>(s) * t;
            lo1 = sext32(static_cast<u32>(sum));
            hi1 = sext32(static_cast<u32>(sum >> 32));
            set64(rd, lo1);
            break;
        }

        case 0x28:  // MMI1
            mmi1(op);
            break;

        case 0x29:  // MMI3
            mmi3(op);
            break;

        case 0x30: {  // PMFHL
            // LO with LO1 and HI with HI1 as two registers, whose words PMFHL picks from.
            const Reg L{lo, lo1}, H{hi, hi1};
            write = true;

            // The shift field chooses the variant (documented).
            switch (shift) {
                case 0:  // LW
                    // The low word of each of LO, HI, LO1 and HI1, in that order.
                    for (unsigned n = 0; n < 2; n++) {
                        put<u32>(out, n * 2, get<u32>(L, n * 2));
                        put<u32>(out, n * 2 + 1, get<u32>(H, n * 2));
                    }

                    break;

                case 1:  // UW
                    // The upper word of each of LO, HI, LO1 and HI1, in that order.
                    for (unsigned n = 0; n < 2; n++) {
                        put<u32>(out, n * 2, get<u32>(L, n * 2 + 1));
                        put<u32>(out, n * 2 + 1, get<u32>(H, n * 2 + 1));
                    }

                    break;

                case 2:  // SLW
                    // Each 64-bit product is saturated to a word.
                    for (unsigned n = 0; n < 2; n++) {
                        s64 v = static_cast<s64>(
                            (static_cast<u64>(get<u32>(H, n * 2)) << 32) | get<u32>(L, n * 2)
                        );
                        put<u64>(out, n, sext32(static_cast<u32>(saturate<s32, s64>(v))));
                    }

                    break;

                case 3:  // LH
                    // The low halfword of each word of LO and HI, then of LO1 and HI1.
                    for (unsigned n = 0; n < 2; n++) {
                        put<u16>(out, n * 4 + 0, get<u16>(L, n * 4 + 0));
                        put<u16>(out, n * 4 + 1, get<u16>(L, n * 4 + 2));
                        put<u16>(out, n * 4 + 2, get<u16>(H, n * 4 + 0));
                        put<u16>(out, n * 4 + 3, get<u16>(H, n * 4 + 2));
                    }

                    break;

                case 4:  // SH
                    // Words are saturated to halfwords, in the order of LH.
                    for (unsigned n = 0; n < 2; n++) {
                        put<s16>(out, n * 4 + 0, saturate<s16, s32>(get<s32>(L, n * 2 + 0)));
                        put<s16>(out, n * 4 + 1, saturate<s16, s32>(get<s32>(L, n * 2 + 1)));
                        put<s16>(out, n * 4 + 2, saturate<s16, s32>(get<s32>(H, n * 2 + 0)));
                        put<s16>(out, n * 4 + 3, saturate<s16, s32>(get<s32>(H, n * 2 + 1)));
                    }

                    break;

                default:
                    // An undefined variant writes nothing and is counted, at address pc - 4.
                    write = false;
                    not_known(op, pc - 4);
                    break;
            }

            break;
        }

        case 0x31: {  // PMTHL (LW)
            // The words of rs fill the low words of LO, HI, LO1, HI1; the rest stay (documented).
            Reg L{lo, lo1}, H{hi, hi1};
            const Reg S = gpr[rs];
            put<u32>(L, 0, get<u32>(S, 0));
            put<u32>(H, 0, get<u32>(S, 1));
            put<u32>(L, 2, get<u32>(S, 2));
            put<u32>(H, 2, get<u32>(S, 3));
            lo = L.lo;
            lo1 = L.hi;
            hi = H.lo;
            hi1 = H.hi;
            break;
        }

        case 0x34:  // PSLLH
            // Halfwords shift by the low four bits of the shift field (documented).
            out = each<u16>(T, T, [&](u16 v, u16) { return v << (shift & 15); });
            write = true;
            break;

        case 0x36:  // PSRLH
            // As PSLLH, but a logical shift right.
            out = each<u16>(T, T, [&](u16 v, u16) { return v >> (shift & 15); });
            write = true;
            break;

        case 0x37:  // PSRAH
            // As PSLLH, but an arithmetic shift right.
            out = each<s16>(T, T, [&](s16 v, s16) { return v >> (shift & 15); });
            write = true;
            break;

        case 0x3C:  // PSLLW
            out = each<u32>(T, T, [&](u32 v, u32) { return v << shift; });
            write = true;
            break;

        case 0x3E:  // PSRLW
            out = each<u32>(T, T, [&](u32 v, u32) { return v >> shift; });
            write = true;
            break;

        case 0x3F:  // PSRAW
            out = each<s32>(T, T, [&](s32 v, s32) { return v >> shift; });
            write = true;
            break;

        default:
            // Unknown code: counted. The address is pc - 4 because pc already points past it.
            not_known(op, pc - 4);
            break;
    }

    // Only the cases that built a vector result write rd, and register 0 is never written.
    if (write && rd) {
        gpr[rd] = out;
    }
}

void Ee::mmi0(u32 op) {
    // Instruction fields: rd bits 11-15, rs bits 21-25, rt bits 16-20 (documented).
    unsigned rd = (op >> 11) & 31;
    const Reg S = gpr[(op >> 21) & 31], T = gpr[(op >> 16) & 31];
    Reg out;

    // The shift field, bits 6-10, chooses the instruction in this group (documented).
    switch ((op >> 6) & 31) {
        case 0x00:  // PADDW
            out = each<u32>(S, T, [](u32 a, u32 b) { return a + b; });
            break;

        case 0x01:  // PSUBW
            out = each<u32>(S, T, [](u32 a, u32 b) { return a - b; });
            break;

        case 0x02:  // PCGTW
            // Each element is all ones where s is greater than t, else 0 (documented).
            out = each<s32>(S, T, [](s32 a, s32 b) { return a > b ? -1 : 0; });
            break;

        case 0x03:  // PMAXW
            out = each<s32>(S, T, [](s32 a, s32 b) { return std::max(a, b); });
            break;

        case 0x04:  // PADDH
            out = each<u16>(S, T, [](u16 a, u16 b) { return a + b; });
            break;

        case 0x05:  // PSUBH
            out = each<u16>(S, T, [](u16 a, u16 b) { return a - b; });
            break;

        case 0x06:  // PCGTH
            out = each<s16>(S, T, [](s16 a, s16 b) { return a > b ? -1 : 0; });
            break;

        case 0x07:  // PMAXH
            out = each<s16>(S, T, [](s16 a, s16 b) { return std::max(a, b); });
            break;

        case 0x08:  // PADDB
            out = each<u8>(S, T, [](u8 a, u8 b) { return a + b; });
            break;

        case 0x09:  // PSUBB
            out = each<u8>(S, T, [](u8 a, u8 b) { return a - b; });
            break;

        case 0x0A:  // PCGTB
            out = each<s8>(S, T, [](s8 a, s8 b) { return a > b ? -1 : 0; });
            break;

        case 0x10:  // PADDSW
            // Signed add that saturates to the word range (documented).
            out = each<s32>(S, T, [](s32 a, s32 b) { return saturate<s32, s64>(s64{a} + b); });
            break;

        case 0x11:  // PSUBSW
            out = each<s32>(S, T, [](s32 a, s32 b) { return saturate<s32, s64>(s64{a} - b); });
            break;

        case 0x12:  // PEXTLW
            out = interleave<u32>(S, T, false);
            break;

        case 0x13:  // PPACW
            out = pack<u32>(S, T);
            break;

        case 0x14:  // PADDSH
            out = each<s16>(S, T, [](s16 a, s16 b) { return saturate<s16, s32>(a + b); });
            break;

        case 0x15:  // PSUBSH
            out = each<s16>(S, T, [](s16 a, s16 b) { return saturate<s16, s32>(a - b); });
            break;

        case 0x16:  // PEXTLH
            out = interleave<u16>(S, T, false);
            break;

        case 0x17:  // PPACH
            out = pack<u16>(S, T);
            break;

        case 0x18:  // PADDSB
            out = each<s8>(S, T, [](s8 a, s8 b) { return saturate<s8, s32>(a + b); });
            break;

        case 0x19:  // PSUBSB
            out = each<s8>(S, T, [](s8 a, s8 b) { return saturate<s8, s32>(a - b); });
            break;

        case 0x1A:  // PEXTLB
            out = interleave<u8>(S, T, false);
            break;

        case 0x1B:  // PPACB
            out = pack<u8>(S, T);
            break;

        case 0x1E:  // PEXT5
            /*
             * 1:5:5:5 to 8:8:8:8 in each word. The red, green and blue fields (bits 0-4, 5-9 and
             * 10-14) move to the top five bits of their bytes and the alpha bit (15) to bit 31
             * (documented).
             */
            out = each<u32>(T, T, [](u32 c, u32) {
                return ((c & 0x1F) << 3) | ((c & 0x3E0) << 6) | ((c & 0x7C00) << 9)
                       | ((c & 0x8000) << 16);
            });
            break;

        case 0x1F:  // PPAC5
            // The reverse of PEXT5.
            out = each<u32>(T, T, [](u32 c, u32) {
                return ((c >> 3) & 0x1F) | ((c >> 6) & 0x3E0) | ((c >> 9) & 0x7C00)
                       | ((c >> 16) & 0x8000);
            });
            break;

        default:
            // Unknown code: counted. The address is pc - 4 because pc already points past it.
            not_known(op, pc - 4);
            return;
    }

    // Register 0 is never written.
    if (rd) {
        gpr[rd] = out;
    }
}

void Ee::mmi1(u32 op) {
    // Instruction fields: rd bits 11-15, rs bits 21-25, rt bits 16-20 (documented).
    unsigned rd = (op >> 11) & 31;
    const Reg S = gpr[(op >> 21) & 31], T = gpr[(op >> 16) & 31];
    Reg out;

    // The shift field, bits 6-10, chooses the instruction in this group (documented).
    switch ((op >> 6) & 31) {
        case 0x01:  // PABSW
            // The absolute value; the most negative number becomes the most positive (documented).
            out = each<s32>(T, T, [](s32 a, s32) {
                return a == INT32_MIN ? INT32_MAX : (a < 0 ? -a : a);
            });
            break;

        case 0x02:  // PCEQW
            // Each element is all ones where s equals t, else 0 (documented).
            out = each<u32>(S, T, [](u32 a, u32 b) { return a == b ? 0xFFFFFFFFu : 0u; });
            break;

        case 0x03:  // PMINW
            out = each<s32>(S, T, [](s32 a, s32 b) { return std::min(a, b); });
            break;

        case 0x04:  // PADSBH
            // The low four halfwords subtract, the high four add.
            for (unsigned n = 0; n < 8; n++) {
                u16 a = get<u16>(S, n), b = get<u16>(T, n);
                put<u16>(out, n, static_cast<u16>(n < 4 ? a - b : a + b));
            }

            break;

        case 0x05:  // PABSH
            out = each<s16>(T, T, [](s16 a, s16) {
                return a == INT16_MIN ? INT16_MAX : (a < 0 ? -a : a);
            });
            break;

        case 0x06:  // PCEQH
            // As PCEQW, with halfwords.
            out = each<u16>(S, T, [](u16 a, u16 b) { return a == b ? 0xFFFF : 0; });
            break;

        case 0x07:  // PMINH
            out = each<s16>(S, T, [](s16 a, s16 b) { return std::min(a, b); });
            break;

        case 0x0A:  // PCEQB
            // As PCEQW, with bytes.
            out = each<u8>(S, T, [](u8 a, u8 b) { return a == b ? 0xFF : 0; });
            break;

        case 0x10:  // PADDUW
            // Unsigned add that saturates at the largest word (documented).
            out = each<u32>(S, T, [](u32 a, u32 b) {
                return static_cast<u32>(std::min<u64>(u64{a} + b, 0xFFFFFFFFu));
            });
            break;

        case 0x11:  // PSUBUW
            // Unsigned subtract that saturates at 0 (documented).
            out = each<u32>(S, T, [](u32 a, u32 b) { return a > b ? a - b : 0; });
            break;

        case 0x12:  // PEXTUW
            out = interleave<u32>(S, T, true);
            break;

        case 0x14:  // PADDUH
            // As PADDUW, saturating at the largest halfword.
            out = each<u16>(S, T, [](u16 a, u16 b) { return std::min<u32>(u32{a} + b, 0xFFFF); });
            break;

        case 0x15:  // PSUBUH
            out = each<u16>(S, T, [](u16 a, u16 b) { return a > b ? a - b : 0; });
            break;

        case 0x16:  // PEXTUH
            out = interleave<u16>(S, T, true);
            break;

        case 0x18:  // PADDUB
            // As PADDUW, saturating at the largest byte.
            out = each<u8>(S, T, [](u8 a, u8 b) { return std::min<u32>(u32{a} + b, 0xFF); });
            break;

        case 0x19:  // PSUBUB
            out = each<u8>(S, T, [](u8 a, u8 b) { return a > b ? a - b : 0; });
            break;

        case 0x1A:  // PEXTUB
            out = interleave<u8>(S, T, true);
            break;

        case 0x1B: {  // QFSRV
            // The 256-bit value rs:rt shifts right by SA, a count of 0-15 bytes (documented).
            unsigned bits_right = (sa & 0xF) * 8;

            // No shift: the result is rt.
            if (bits_right == 0) {
                out = T;
            } else if (bits_right < 64) {
                // Less than half a register: rt's two halves move down and rs's low half fills in.
                out.lo = (T.lo >> bits_right) | (T.hi << (64 - bits_right));
                out.hi = (T.hi >> bits_right) | (S.lo << (64 - bits_right));
            } else {
                /*
                 * Half a register or more: rt's high half and rs move down. A shift by 64 would be
                 * undefined in C++, so the ternaries pick the plain move for it.
                 */
                unsigned n = bits_right - 64;
                out.lo = n ? (T.hi >> n) | (S.lo << (64 - n)) : T.hi;
                out.hi = n ? (S.lo >> n) | (S.hi << (64 - n)) : S.lo;
            }

            break;
        }

        default:
            // Unknown code: counted. The address is pc - 4 because pc already points past it.
            not_known(op, pc - 4);
            return;
    }

    // Register 0 is never written.
    if (rd) {
        gpr[rd] = out;
    }
}

void Ee::mmi2(u32 op) {
    // Instruction fields: rd bits 11-15, rs bits 21-25, rt bits 16-20 (documented).
    unsigned rd = (op >> 11) & 31;
    const Reg S = gpr[(op >> 21) & 31], T = gpr[(op >> 16) & 31];
    Reg out;

    // Whether rd receives `out`; the divides and some multiplies leave rd alone.
    bool write = true;

    // The shift field, bits 6-10, chooses the instruction in this group (documented).
    switch ((op >> 6) & 31) {
        case 0x00:    // PMADDW
        case 0x04: {  // PMSUBW
            // Word 0 of each source works on HI:LO and word 2 on HI1:LO1. rd gets both results.
            bool subtract = ((op >> 6) & 31) == 0x04;
            u64* his[2] = {&hi, &hi1};
            u64* los[2] = {&lo, &lo1};

            // One round for each multiply unit.
            for (unsigned n = 0; n < 2; n++) {
                s64 product = static_cast<s64>(get<s32>(S, n * 2)) * get<s32>(T, n * 2);
                s64 base = static_cast<s64>((*his[n] << 32) | (*los[n] & 0xFFFFFFFFu));
                s64 sum = subtract ? base - product : base + product;
                *los[n] = sext32(static_cast<u32>(sum));
                *his[n] = sext32(static_cast<u32>(sum >> 32));
                put<u64>(out, n, static_cast<u64>(sum));
            }

            break;
        }

        case 0x02:  // PSLLVW
            // Words 0 and 2 of rt shift by the low five bits of rs's words; results are
            // sign-extended (documented).
            out.lo = sext32(get<u32>(T, 0) << (get<u32>(S, 0) & 31));
            out.hi = sext32(get<u32>(T, 2) << (get<u32>(S, 2) & 31));
            break;

        case 0x03:  // PSRLVW
            // As PSLLVW, but a logical shift right.
            out.lo = sext32(get<u32>(T, 0) >> (get<u32>(S, 0) & 31));
            out.hi = sext32(get<u32>(T, 2) >> (get<u32>(S, 2) & 31));
            break;

        case 0x08:  // PMFHI
            out = Reg{hi, hi1};
            break;

        case 0x09:  // PMFLO
            out = Reg{lo, lo1};
            break;

        case 0x0A:  // PINTH
            // Interleaves t's low halfwords with s's high ones.
            for (unsigned n = 0; n < 4; n++) {
                put<u16>(out, n * 2, get<u16>(T, n));
                put<u16>(out, n * 2 + 1, get<u16>(S, 4 + n));
            }

            break;

        case 0x0C: {  // PMULTW
            // As PMADDW without the sum: the products replace HI:LO and HI1:LO1.
            u64* his[2] = {&hi, &hi1};
            u64* los[2] = {&lo, &lo1};

            // One round for each multiply unit.
            for (unsigned n = 0; n < 2; n++) {
                s64 product = static_cast<s64>(get<s32>(S, n * 2)) * get<s32>(T, n * 2);
                *los[n] = sext32(static_cast<u32>(product));
                *his[n] = sext32(static_cast<u32>(product >> 32));
                put<u64>(out, n, static_cast<u64>(product));
            }

            break;
        }

        case 0x0D: {  // PDIVW
            // Word 0 divides into LO and HI, word 2 into LO1 and HI1. rd is not written.
            Division a = divide(get<s32>(S, 0), get<s32>(T, 0)),
                     b = divide(get<s32>(S, 2), get<s32>(T, 2));
            lo = sext32(a.quotient);
            hi = sext32(a.remainder);
            lo1 = sext32(b.quotient);
            hi1 = sext32(b.remainder);
            write = false;
            break;
        }

        case 0x0E:  // PCPYLD
            out.lo = T.lo;
            out.hi = S.lo;
            break;

        case 0x10:    // PMADDH
        case 0x14:    // PMSUBH
        case 0x1C: {  // PMULTH
            // The shift field again, to tell the three instructions apart.
            unsigned kind = (op >> 6) & 31;
            Reg L{lo, lo1}, H{hi, hi1};

            // Products 0, 1 go to LO's words, 2, 3 to HI's, 4, 5 to LO1's, 6, 7 to HI1's
            // (documented).
            for (unsigned n = 0; n < 8; n++) {
                s32 product = static_cast<s32>(get<s16>(S, n)) * get<s16>(T, n);

                // Bit 1 of n picks LO or HI, bit 2 picks the unit, and bit 0 picks the word.
                Reg& where = (n & 2) ? H : L;
                unsigned word = (n & 4 ? 2 : 0) + (n & 1);
                s32 old = get<s32>(where, word);

                // PMULTH stores the product, PMADDH adds it and PMSUBH subtracts it, wrapping.
                s32 value =
                    kind == 0x1C
                        ? product
                        : static_cast<s32>(
                              kind == 0x10 ? static_cast<u32>(old) + static_cast<u32>(product)
                                           : static_cast<u32>(old) - static_cast<u32>(product)
                          );
                put<s32>(where, word, value);

                // Only the even products go to rd, in order.
                if ((n & 1) == 0) {
                    put<s32>(out, n / 2, value);
                }
            }

            lo = L.lo;
            lo1 = L.hi;
            hi = H.lo;
            hi1 = H.hi;
            break;
        }

        case 0x11:    // PHMADH
        case 0x15: {  // PHMSBH
            // PHMSBH subtracts; PHMADH adds.
            bool subtract = ((op >> 6) & 31) == 0x15;

            // Each word is the odd halfword product plus the even one, or minus it for PHMSBH.
            for (unsigned n = 0; n < 4; n++) {
                s32 odd = static_cast<s32>(get<s16>(S, n * 2 + 1)) * get<s16>(T, n * 2 + 1);
                s32 even = static_cast<s32>(get<s16>(S, n * 2)) * get<s16>(T, n * 2);
                put<s32>(
                    out,
                    n,
                    static_cast<s32>(
                        subtract ? static_cast<u32>(odd) - static_cast<u32>(even)
                                 : static_cast<u32>(odd) + static_cast<u32>(even)
                    )
                );
            }

            // The four sums also go to the low words of LO, HI, LO1 and HI1; the upper words stay.
            lo = (lo & 0xFFFFFFFF00000000ull) | get<u32>(out, 0);
            hi = (hi & 0xFFFFFFFF00000000ull) | get<u32>(out, 1);
            lo1 = (lo1 & 0xFFFFFFFF00000000ull) | get<u32>(out, 2);
            hi1 = (hi1 & 0xFFFFFFFF00000000ull) | get<u32>(out, 3);
            break;
        }

        case 0x12:  // PAND
            out = Reg{S.lo & T.lo, S.hi & T.hi};
            break;

        case 0x13:  // PXOR
            out = Reg{S.lo ^ T.lo, S.hi ^ T.hi};
            break;

        case 0x1A: {  // PEXEH
            // Swaps halfwords 0 and 2 in each half of rt.
            static constexpr unsigned order[8] = {2, 1, 0, 3, 6, 5, 4, 7};
            out = shuffle<u16>(T, order);
            break;
        }

        case 0x1B: {  // PREVH
            // Reverses the four halfwords in each half of rt.
            static constexpr unsigned order[8] = {3, 2, 1, 0, 7, 6, 5, 4};
            out = shuffle<u16>(T, order);
            break;
        }

        case 0x1D: {  // PDIVBW
            // Every word of s divides by t's first halfword; quotients go to LO and LO1,
            // remainders to HI and HI1.
            s32 d = get<s16>(T, 0);
            Reg L, H;

            // One division for each of the four words of s.
            for (unsigned n = 0; n < 4; n++) {
                Division q = divide(get<s32>(S, n), d);
                put<u32>(L, n, q.quotient);

                // The remainder is cut to 16 bits and sign-extended.
                put<u32>(H, n, static_cast<u32>(static_cast<s32>(static_cast<s16>(q.remainder))));
            }

            lo = L.lo;
            lo1 = L.hi;
            hi = H.lo;
            hi1 = H.hi;
            write = false;
            break;
        }

        case 0x1E: {  // PEXEW
            // Swaps words 0 and 2 of rt.
            static constexpr unsigned order[4] = {2, 1, 0, 3};
            out = shuffle<u32>(T, order);
            break;
        }

        case 0x1F: {  // PROT3W
            // Rotates the three low words of rt: word 1 moves to 0, word 2 to 1, word 0 to 2.
            static constexpr unsigned order[4] = {1, 2, 0, 3};
            out = shuffle<u32>(T, order);
            break;
        }

        default:
            // Unknown code: counted. The address is pc - 4 because pc already points past it.
            not_known(op, pc - 4);
            return;
    }

    // Register 0 is never written, and the divides did not make a result for rd.
    if (write && rd) {
        gpr[rd] = out;
    }
}

void Ee::mmi3(u32 op) {
    // Instruction fields: rd bits 11-15, rs bits 21-25, rt bits 16-20 (documented).
    unsigned rd = (op >> 11) & 31;
    const Reg S = gpr[(op >> 21) & 31], T = gpr[(op >> 16) & 31];
    Reg out;

    // Whether rd receives `out`; the moves to HI and LO and the divide leave rd alone.
    bool write = true;

    // The shift field, bits 6-10, chooses the instruction in this group (documented).
    switch ((op >> 6) & 31) {
        case 0x00:    // PMADDUW
        case 0x0C: {  // PMULTUW
            // Word 0 of each source works on HI:LO and word 2 on HI1:LO1. rd gets both results.
            bool add = ((op >> 6) & 31) == 0x00;
            u64* his[2] = {&hi, &hi1};
            u64* los[2] = {&lo, &lo1};

            // One round for each multiply unit.
            for (unsigned n = 0; n < 2; n++) {
                u64 product = static_cast<u64>(get<u32>(S, n * 2)) * get<u32>(T, n * 2);

                // PMADDUW adds the unit's old 64-bit value; PMULTUW replaces it.
                if (add) {
                    product += (*his[n] << 32) | (*los[n] & 0xFFFFFFFFu);
                }

                *los[n] = sext32(static_cast<u32>(product));
                *his[n] = sext32(static_cast<u32>(product >> 32));
                put<u64>(out, n, product);
            }

            break;
        }

        case 0x03:  // PSRAVW
            // Arithmetic shift right of words 0 and 2 of rt by rs's low five bits; results are
            // sign-extended (documented).
            out.lo = sext32(static_cast<u32>(get<s32>(T, 0) >> (get<u32>(S, 0) & 31)));
            out.hi = sext32(static_cast<u32>(get<s32>(T, 2) >> (get<u32>(S, 2) & 31)));
            break;

        case 0x08:  // PMTHI
            hi = S.lo;
            hi1 = S.hi;
            write = false;
            break;

        case 0x09:  // PMTLO
            lo = S.lo;
            lo1 = S.hi;
            write = false;
            break;

        case 0x0A:  // PINTEH
            // Interleaves the even halfwords of t and s.
            for (unsigned n = 0; n < 4; n++) {
                put<u16>(out, n * 2, get<u16>(T, n * 2));
                put<u16>(out, n * 2 + 1, get<u16>(S, n * 2));
            }

            break;

        case 0x0D: {  // PDIVUW
            // Word 0 divides into LO and HI, word 2 into LO1 and HI1. rd is not written.
            Division a = divide_unsigned(get<u32>(S, 0), get<u32>(T, 0)),
                     b = divide_unsigned(get<u32>(S, 2), get<u32>(T, 2));
            lo = sext32(a.quotient);
            hi = sext32(a.remainder);
            lo1 = sext32(b.quotient);
            hi1 = sext32(b.remainder);
            write = false;
            break;
        }

        case 0x0E:  // PCPYUD
            out.lo = S.hi;
            out.hi = T.hi;
            break;

        case 0x12:  // POR
            out = Reg{S.lo | T.lo, S.hi | T.hi};
            break;

        case 0x13:  // PNOR
            out = Reg{~(S.lo | T.lo), ~(S.hi | T.hi)};
            break;

        case 0x1A: {  // PEXCH
            // Swaps halfwords 1 and 2 in each half of rt.
            static constexpr unsigned order[8] = {0, 2, 1, 3, 4, 6, 5, 7};
            out = shuffle<u16>(T, order);
            break;
        }

        case 0x1B:  // PCPYH
            // The first halfword of each half, four times.
            for (unsigned n = 0; n < 8; n++) {
                put<u16>(out, n, get<u16>(T, n & 4));
            }

            break;

        case 0x1E: {  // PEXCW
            // Swaps words 1 and 2 of rt.
            static constexpr unsigned order[4] = {0, 2, 1, 3};
            out = shuffle<u32>(T, order);
            break;
        }

        default:
            // Unknown code: counted. The address is pc - 4 because pc already points past it.
            not_known(op, pc - 4);
            return;
    }

    // Register 0 is never written, and the moves and the divide made no result for rd.
    if (write && rd) {
        gpr[rd] = out;
    }
}

}  // namespace ps2
