// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The console's arithmetic (fp.h) on four fields at once, in the host's floating point where that
 * gives the same bits.
 *
 * With the host rounding towards zero, a product of two ordinary numbers is the console's product,
 * and a sum is the console's sum once the smaller operand has lost the bits the console's adder
 * drops. That leaves the special values: operands with the smallest or largest exponent, and
 * results that leave the ordinary range. Each function works out which fields are ordinary; when a
 * field the caller needs is not, it returns false and the caller computes the instruction with fp.h
 * instead. The two agree on every case that `tests/test_vu.cpp` tries (measured).
 *
 * The caller must have set the host's rounding to "towards zero" (want_toward_zero).
 *
 * Without NEON (any host that is not Arm 64) the four-field functions always return false.
 *
 * Sources: the Arm floating-point control register as publicly documented, and fp.h.
 */

#pragma once

#include <cfenv>

#include "fp.h"
#include "types.h"

// Only Arm 64 hosts have the NEON path; the others get stand-ins that return false.
#if defined(__aarch64__)
#include <arm_neon.h>

/** NEON is there: the four-field functions compute. */
#define OPENRAC_FP_QUAD 1
#else
/** NEON is not there: the four-field functions only return false. */
#define OPENRAC_FP_QUAD 0
#endif

namespace ps2::fp {

// On an Arm host the rounding mode is read and set directly in FPCR (documented).
#if defined(__aarch64__)

/**
 * Reads the host's floating-point control register.
 *
 * The C library's way of reading the mode is slow, and this is called around every packet a vector
 * unit sends.
 *
 * @return The raw register, to hand back to `set_host_rounding`.
 */
inline u64 host_rounding() {
    return __builtin_arm_rsr64("FPCR");
}

/**
 * Restores the host's floating-point control register.
 *
 * @param saved A value `host_rounding` returned.
 */
inline void set_host_rounding(u64 saved) {
    __builtin_arm_wsr64("FPCR", saved);
}

/** Sets the host's rounding to "towards zero", whatever it was. */
inline void round_toward_zero() {
    // FPCR bits 22-23 are the rounding mode, and 3 is "towards zero" (documented).
    __builtin_arm_wsr64("FPCR", __builtin_arm_rsr64("FPCR") | (u64{3} << 22));
}

/** Sets the host's rounding to "to nearest", whatever it was. */
inline void round_to_nearest() {
    // FPCR bits 22-23 are the rounding mode, and 0 is "to nearest" (documented).
    __builtin_arm_wsr64("FPCR", __builtin_arm_rsr64("FPCR") & ~(u64{3} << 22));
}

/**
 * Sets the host's rounding to "towards zero" if it is not set already.
 *
 * Changing the mode is slow and reading it is not, so code that needs one mode asks for it on entry
 * and nobody switches back: the vector units ask for "towards zero", whatever computes as the host
 * does (the GS, the program around the machine) asks for "to nearest".
 */
inline void want_toward_zero() {
    u64 mode = __builtin_arm_rsr64("FPCR");

    // FPCR bits 22-23 are the rounding mode, and 3 is "towards zero" (documented).
    if ((mode & (u64{3} << 22)) != (u64{3} << 22)) {
        __builtin_arm_wsr64("FPCR", mode | (u64{3} << 22));
    }
}

/**
 * Sets the host's rounding to "to nearest" if it is not set already.
 *
 * See `want_toward_zero` for why the mode is asked for and not switched back.
 */
inline void want_nearest() {
    u64 mode = __builtin_arm_rsr64("FPCR");

    // FPCR bits 22-23 are the rounding mode, and 0 is "to nearest" (documented).
    if (mode & (u64{3} << 22)) {
        __builtin_arm_wsr64("FPCR", mode & ~(u64{3} << 22));
    }
}

// Other hosts use the C library's rounding functions.
#else

/**
 * Reads the host's rounding mode.
 *
 * @return The mode as the C library gives it, to hand back to `set_host_rounding`.
 */
inline u64 host_rounding() {
    return static_cast<u64>(std::fegetround());
}

/**
 * Restores the host's rounding mode.
 *
 * @param saved A value `host_rounding` returned.
 */
inline void set_host_rounding(u64 saved) {
    std::fesetround(static_cast<int>(saved));
}

/** Sets the host's rounding to "towards zero", whatever it was. */
inline void round_toward_zero() {
    std::fesetround(FE_TOWARDZERO);
}

/** Sets the host's rounding to "to nearest", whatever it was. */
inline void round_to_nearest() {
    std::fesetround(FE_TONEAREST);
}

/**
 * Sets the host's rounding to "towards zero" if it is not set already.
 *
 * See the Arm version above for why the mode is asked for and not switched back.
 */
inline void want_toward_zero() {
    // Already set: leave the mode alone, since changing it is the costly part.
    if (std::fegetround() != FE_TOWARDZERO) {
        std::fesetround(FE_TOWARDZERO);
    }
}

/** Sets the host's rounding to "to nearest" if it is not set already. */
inline void want_nearest() {
    // Already set: leave the mode alone, since changing it is the costly part.
    if (std::fegetround() != FE_TONEAREST) {
        std::fesetround(FE_TONEAREST);
    }
}

#endif

// NEON hosts get the real four-field functions; the others get stand-ins further down.
#if OPENRAC_FP_QUAD

namespace quad_detail {

/**
 * Takes the exponent field of each of four patterns.
 *
 * @param v Four patterns.
 * @return Four exponents, 0 to 255, one per field.
 */
inline uint32x4_t exponent(uint32x4_t v) {
    // The exponent is bits 23-30 of a pattern (documented).
    return vandq_u32(vshrq_n_u32(v, 23), vdupq_n_u32(0xFF));
}

/**
 * Marks the fields that are ordinary numbers or plain zeros.
 *
 * @param v Four patterns.
 * @param e The exponents of `v`, from `exponent`.
 * @return All ones in the fields that are ordinary numbers or plain zeros, otherwise 0.
 */
inline uint32x4_t usable(uint32x4_t v, uint32x4_t e) {
    // Exponents 1 to 254 mean the same on the host and the console; 0 and 255 do not.
    uint32x4_t ordinary = vandq_u32(vcgtq_u32(e, vdupq_n_u32(0)), vcltq_u32(e, vdupq_n_u32(255)));

    // A pattern with nothing but a sign bit is a zero on both, whatever its exponent field says.
    uint32x4_t zero = vceqq_u32(vandq_u32(v, vdupq_n_u32(0x7FFFFFFFu)), vdupq_n_u32(0));

    return vorrq_u32(ordinary, zero);
}

/**
 * Turns a dest mask into a mask over the four fields.
 *
 * @param dest The dest bits of an instruction, x in bit 3 down to w in bit 0.
 * @return The dest mask (x is bit 3) as all ones in the fields it names.
 */
inline uint32x4_t fields(u32 dest) {
    // Dest bit values for the fields x, y, z and w (documented).
    const uint32x4_t bit = {8, 4, 2, 1};
    return vtstq_u32(vdupq_n_u32(dest), bit);
}

/**
 * Tells whether a field mask is empty.
 *
 * @param v A mask over four fields.
 * @return True when no bit is set in any field.
 */
inline bool none(uint32x4_t v) {
    return vmaxvq_u32(v) == 0;
}

}  // namespace quad_detail

/**
 * Multiplies four fields at once on the host: out = a * b in the fields of `dest`.
 *
 * When it returns false, nothing was written and the caller uses fp::mul for this instruction.
 *
 * @param a Four patterns, x first.
 * @param b Four patterns, x first.
 * @param dest The dest bits of the instruction, x in bit 3 down to w in bit 0.
 * @param[out] out Four patterns; only the fields of `dest` are meaningful, and only on success.
 * @return True when every field of `dest` is a case the host computes as the console does.
 */
inline bool quad_mul(const u32* a, const u32* b, u32 dest, u32* out) {
    using namespace quad_detail;
    uint32x4_t va = vld1q_u32(a), vb = vld1q_u32(b);
    uint32x4_t ea = exponent(va), eb = exponent(vb);

    /*
     * The result's exponent is the sum of the two, or one more: keep well inside the range where it
     * is neither too small nor too large.
     */
    uint32x4_t sum = vaddq_u32(ea, eb);

    // Each bound is the bias 127 plus the product exponent's limit: above 1, below 253.
    uint32x4_t in_range =
        vandq_u32(vcgtq_u32(sum, vdupq_n_u32(127 + 1)), vcltq_u32(sum, vdupq_n_u32(127 + 253)));

    // A zero operand gives a zero product whatever the other exponent, so the range test is waived.
    uint32x4_t either_zero = vorrq_u32(
        vceqq_u32(vandq_u32(va, vdupq_n_u32(0x7FFFFFFFu)), vdupq_n_u32(0)),
        vceqq_u32(vandq_u32(vb, vdupq_n_u32(0x7FFFFFFFu)), vdupq_n_u32(0))
    );

    // A field is fine when both operands are usable and the product is in range or a zero.
    uint32x4_t ok =
        vandq_u32(vandq_u32(usable(va, ea), usable(vb, eb)), vorrq_u32(in_range, either_zero));

    // A field the caller needs is not fine: leave the instruction to fp::mul.
    if (!none(vbicq_u32(fields(dest), ok))) {
        return false;
    }

    // The host product, with rounding towards zero, is the console's product in these fields.
    vst1q_u32(
        out, vreinterpretq_u32_f32(vmulq_f32(vreinterpretq_f32_u32(va), vreinterpretq_f32_u32(vb)))
    );
    return true;
}

/**
 * Adds or subtracts four fields at once on the host: out = a + b (or a - b) in the fields of
 * `dest`.
 *
 * When it returns false, nothing was written and the caller uses fp::add for this instruction.
 *
 * @param a Four patterns, x first.
 * @param b Four patterns, x first.
 * @param dest The dest bits of the instruction, x in bit 3 down to w in bit 0.
 * @param[out] out Four patterns; only the fields of `dest` are meaningful, and only on success.
 * @param subtract True to compute a - b.
 * @return True when every field of `dest` is a case the host computes as the console does.
 */
inline bool quad_add(const u32* a, const u32* b, u32 dest, u32* out, bool subtract = false) {
    using namespace quad_detail;
    uint32x4_t va = vld1q_u32(a), vb = vld1q_u32(b);

    // A difference is the sum with the second operand's sign bit flipped.
    if (subtract) {
        vb = veorq_u32(vb, vdupq_n_u32(kSign));
    }

    uint32x4_t ea = exponent(va), eb = exponent(vb);

    /*
     * The operand with the smaller exponent keeps one bit below the other's last place; with
     * exponents 25 or more apart it does not count at all.
     */
    uint32x4_t a_small = vcltq_u32(ea, eb);
    uint32x4_t hi = vbslq_u32(a_small, vb, va), lo = vbslq_u32(a_small, va, vb);
    uint32x4_t ehi = vbslq_u32(a_small, eb, ea);
    uint32x4_t d = vsubq_u32(ehi, vbslq_u32(a_small, ea, eb));

    // (A shift count of 32 or more gives zero, which clears the operand.)
    int32x4_t drop = vreinterpretq_s32_u32(vqsubq_u32(d, vdupq_n_u32(1)));
    uint32x4_t mask = vshlq_u32(vdupq_n_u32(0xFFFFFFFFu), drop);

    // A gap above 24 clears the smaller operand: it does not count at all.
    mask = vbslq_u32(vcgtq_u32(d, vdupq_n_u32(24)), vdupq_n_u32(0), mask);
    lo = vandq_u32(lo, mask);
    uint32x4_t r =
        vreinterpretq_u32_f32(vaddq_f32(vreinterpretq_f32_u32(hi), vreinterpretq_f32_u32(lo)));

    /*
     * Ordinary operands, no risk of leaving the top of the range, and a result that is an ordinary
     * number or an exact zero.
     */
    uint32x4_t er = exponent(r);
    uint32x4_t result_ok = vorrq_u32(vcgtq_u32(er, vdupq_n_u32(0)), vceqq_u32(r, vdupq_n_u32(0)));

    // A larger operand below exponent 254 cannot carry into 255, where the host overflows.
    uint32x4_t ok = vandq_u32(
        vandq_u32(usable(va, ea), usable(vb, eb)),
        vandq_u32(vcltq_u32(ehi, vdupq_n_u32(254)), result_ok)
    );

    // A field the caller needs is not fine: leave the instruction to fp::add.
    if (!none(vbicq_u32(fields(dest), ok))) {
        return false;
    }

    vst1q_u32(out, r);
    return true;
}

/**
 * Copies the fields of `dest` from one quadword to another: to = from in the fields of `dest`.
 *
 * @param[out] to Four patterns; the fields not in `dest` keep their value.
 * @param from Four patterns, x first.
 * @param dest The dest bits of the instruction, x in bit 3 down to w in bit 0.
 */
inline void quad_merge(u32* to, const u32* from, u32 dest) {
    vst1q_u32(to, vbslq_u32(quad_detail::fields(dest), vld1q_u32(from), vld1q_u32(to)));
}

// Without NEON there is no fast path: the stand-ins below send every instruction to fp.h.
#else

/**
 * Copies the fields of `dest` from one quadword to another: to = from in the fields of `dest`.
 *
 * @param[out] to Four patterns; the fields not in `dest` keep their value.
 * @param from Four patterns, x first.
 * @param dest The dest bits of the instruction, x in bit 3 down to w in bit 0.
 */
inline void quad_merge(u32* to, const u32* from, u32 dest) {
    // The four fields in order, x first.
    for (unsigned field = 0; field < 4; field++) {
        // Dest bit 3 is x and bit 0 is w: the field's bit is 8 shifted down by its number.
        if (dest & (8u >> field)) {
            to[field] = from[field];
        }
    }
}

/**
 * Stands in for the NEON `quad_mul`: it computes nothing, and its four parameters are unused.
 *
 * @return Always false, so the caller computes the instruction with fp::mul.
 */
inline bool quad_mul(const u32*, const u32*, u32, u32*) {
    return false;
}

/**
 * Stands in for the NEON `quad_add`: it computes nothing, and its five parameters are unused.
 *
 * @return Always false, so the caller computes the instruction with fp::add.
 */
inline bool quad_add(const u32*, const u32*, u32, u32*, bool = false) {
    return false;
}

#endif

}  // namespace ps2::fp
