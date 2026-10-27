// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * Single-precision arithmetic as the console's FPU and vector units do it, on raw 32-bit patterns,
 * so that results do not depend on the host:
 *
 * - an exponent of 0 is zero, whatever the fraction (no denormals);
 * - an exponent of 255 is an ordinary large number (no infinities or NaNs);
 * - results are cut to 24 significant bits, never rounded up;
 * - a result too large is the largest number, 0x7FFFFFFF with its sign, and one too small is zero;
 * - the adder keeps one bit of the smaller operand below the larger one's last place and drops the
 *   rest before it adds.
 *
 * The rules are the ones ReRAC's `tfrag_light::ps2` module checked against a running game's
 * lighting output (ISC, https://github.com/re-rac/rerac); this is an independent implementation of
 * them. Rare last-bit deviations of the hardware multiplier are not modelled.
 *
 * Sources: the ReRAC rules above; a fact in this file tagged (documented) comes from them.
 */

#pragma once

#include <bit>
#include <cmath>

#include "types.h"

namespace ps2::fp {

/**
 * The sign bit, the largest number (0x7FFFFFFF, sign clear) and 1.0f, as bit patterns.
 */
constexpr u32 kSign = 0x80000000u, kMax = 0x7FFFFFFFu, kOne = 0x3F800000u;

/**
 * What went wrong, for the status flags: one bit each, ORed into the `flags` of the operations.
 */
enum : u32 {
    kOverflow = 1,
    kUnderflow = 2,
    kDivideByZero = 4,
    kInvalid = 8
};

/**
 * Returns the biased exponent of a pattern.
 *
 * @param x A pattern.
 * @return The exponent field, 0 to 255.
 */
constexpr s32 exponent(u32 x) {
    // The exponent field is bits 23-30 of the pattern (documented).
    return static_cast<s32>((x >> 23) & 0xFF);
}

/**
 * Returns the 24-bit significand of a pattern, with its implied leading one.
 *
 * @param x A pattern whose exponent is not 0.
 * @return The fraction (bits 0-22) with bit 23 set.
 */
constexpr u64 mantissa(u32 x) {
    // Fraction bits 0-22, plus the leading one at bit 23 (documented).
    return (x & 0x7FFFFFu) | 0x800000u;
}

/**
 * Tells whether a pattern is a zero.
 *
 * @param x A pattern.
 * @return True when its exponent is 0, whatever its fraction and sign.
 */
constexpr bool is_zero(u32 x) {
    return exponent(x) == 0;
}

/**
 * Builds a pattern from a sign, a magnitude and a unit, cutting the magnitude to 24 bits.
 *
 * The value is sign * magnitude * 2^(unit - 150): `unit` is the biased exponent a magnitude of
 * exactly 2^23 would have.
 *
 * @param sign Either 0 or kSign.
 * @param magnitude The unsigned size, counted in units of 2^(unit - 150).
 * @param unit The biased exponent of a magnitude of 2^23.
 * @param[out] flags Receives kOverflow or kUnderflow, ORed in, when the result leaves the range.
 * @return The pattern: the largest number on overflow, zero on underflow, both with the sign.
 */
constexpr u32 pack(u32 sign, u64 magnitude, s32 unit, u32& flags) {
    // A magnitude of 0 is a zero of the given sign.
    if (magnitude == 0) {
        return sign;
    }

    // The leading one of the 24-bit significand sits at bit 23; 63 is the top bit of a u64.
    s32 top = 63 - std::countl_zero(magnitude);
    s32 e = unit + (top - 23);

    // Bits below the 24 kept are dropped, so the result is cut and never rounded (documented).
    u64 m = top >= 23 ? magnitude >> (top - 23) : magnitude << (23 - top);

    // A biased exponent past 255 is too large: the largest number with the sign (documented).
    if (e > 255) {
        flags |= kOverflow;
        return sign | kMax;
    }

    // A biased exponent below 1 is too small: a zero with the sign (documented).
    if (e < 1) {
        flags |= kUnderflow;
        return sign;
    }

    // Exponent in bits 23-30, fraction in bits 0-22; the leading one is implied (documented).
    return sign | (static_cast<u32>(e) << 23) | (static_cast<u32>(m) & 0x7FFFFFu);
}

/**
 * Multiplies two numbers.
 *
 * @param a The first factor.
 * @param b The second factor.
 * @param[out] flags Receives the flags `pack` raises, ORed in.
 * @return The product.
 */
constexpr u32 mul(u32 a, u32 b, u32& flags) {
    // The product is negative when exactly one factor is.
    u32 sign = (a ^ b) & kSign;

    // A zero factor gives a zero.
    if (is_zero(a) || is_zero(b)) {
        return sign;
    }

    // The two significands multiply into 47 or 48 bits; 127 is the exponent bias, counted once.
    return pack(sign, (mantissa(a) * mantissa(b)) >> 23, exponent(a) + exponent(b) - 127, flags);
}

/**
 * Adds two numbers, with the console's adder: it keeps one bit of the smaller below the larger.
 *
 * @param a The first addend.
 * @param b The second addend.
 * @param[out] flags Receives the flags `pack` raises, ORed in.
 * @return The sum.
 */
constexpr u32 add(u32 a, u32 b, u32& flags) {
    // A zero operand leaves the other one as it is.
    if (is_zero(a) || is_zero(b)) {
        // Only b is zero, so the sum is a.
        if (!is_zero(a)) {
            return a;
        }

        // Only a is zero, so the sum is b.
        if (!is_zero(b)) {
            return b;
        }

        // Both are zero: the sign survives only when both operands have it.
        return a & b & kSign;
    }

    // Order by exponent: `hi` has the larger one, `lo` loses bits, `d` is the gap.
    u32 hi = exponent(a) >= exponent(b) ? a : b;
    u32 lo = exponent(a) >= exponent(b) ? b : a;
    s32 d = exponent(hi) - exponent(lo);

    // A smaller operand 25 or more exponents below does not count at all (documented).
    if (d >= 25) {
        return hi;
    }

    // Clear the bits of lo more than one below hi's last place (documented).
    if (d >= 1) {
        lo &= 0xFFFFFFFFu << (d - 1);
    }

    // Signed significands on lo's scale: hi shifts up by the gap, and a set sign negates.
    s64 big = static_cast<s64>(mantissa(hi) << d), small = static_cast<s64>(mantissa(lo));
    s64 sum = ((hi & kSign) ? -big : big) + ((lo & kSign) ? -small : small);

    // The operands cancelled exactly: the result is a positive zero.
    if (sum == 0) {
        return 0;
    }

    // Back to sign and magnitude, counted in lo's last place, so lo's exponent is the unit.
    return pack(sum < 0 ? kSign : 0, static_cast<u64>(sum < 0 ? -sum : sum), exponent(lo), flags);
}

/**
 * Subtracts one number from another.
 *
 * @param a The minuend.
 * @param b The subtrahend.
 * @param[out] flags Receives the flags `add` raises, ORed in.
 * @return The difference.
 */
constexpr u32 sub(u32 a, u32 b, u32& flags) {
    // A difference is the sum with the second operand's sign bit flipped.
    return add(a, b ^ kSign, flags);
}

/**
 * Divides one number by another.
 *
 * @param a The dividend.
 * @param b The divisor.
 * @param[out] flags Receives kInvalid or kDivideByZero for a zero divisor, and what `pack` raises.
 * @return The quotient, cut; the largest number with the sign when the divisor is zero.
 */
constexpr u32 div(u32 a, u32 b, u32& flags) {
    // The quotient is negative when exactly one operand is.
    u32 sign = (a ^ b) & kSign;

    // A zero divisor gives the largest number with the sign (documented).
    if (is_zero(b)) {
        // 0 / 0 is invalid; any other number over zero is a divide by zero.
        flags |= is_zero(a) ? kInvalid : kDivideByZero;
        return sign | kMax;
    }

    // A zero dividend over a nonzero divisor is a zero.
    if (is_zero(a)) {
        return sign;
    }

    // The dividend is shifted left 24 for precision, which moves the unit from 150 to 126.
    return pack(sign, (mantissa(a) << 24) / mantissa(b), exponent(a) - exponent(b) + 126, flags);
}

/**
 * Returns the integer square root of a 64-bit number, rounded down.
 *
 * @param n The number.
 * @return The largest r with r * r <= n; it fits in 32 bits.
 */
constexpr u64 isqrt(u64 n) {
    u64 r = 0;

    // Try each bit of the root from bit 31 down, and keep it when the square still fits.
    for (u64 bit = u64{1} << 31; bit; bit >>= 1) {
        u64 t = r | bit;

        // The root with this bit set is still not above the square root of n.
        if (t * t <= n) {
            r = t;
        }
    }

    return r;
}

/**
 * Returns the square root of the magnitude; a negative operand is flagged.
 *
 * @param x The operand.
 * @param[out] flags Receives kInvalid, ORed in, when `x` is negative.
 * @return The root, cut to 24 bits; 0 for a zero operand.
 */
constexpr u32 sqrt(u32 x, u32& flags) {
    // The root of zero is zero.
    if (is_zero(x)) {
        return 0;
    }

    // A negative operand is invalid, but the root of its magnitude is still taken (documented).
    if (x & kSign) {
        flags |= kInvalid;
    }

    // The unbiased exponent, and its parity; an odd one is folded into the significand.
    s32 e = exponent(x) - 127;
    s32 odd = ((e % 2) + 2) % 2;

    // A normal number's root cannot leave the range, so the flags of `pack` are not kept.
    u32 unused = 0;

    // The significand shifts left 23 plus the odd bit; the exponent halves; 127 is the bias.
    return pack(0, isqrt(mantissa(x) << (23 + odd)), (e - odd) / 2 + 127, unused);
}

/**
 * Returns a / sqrt(|b|): the root is cut first, then the division.
 *
 * @param a The dividend.
 * @param b The operand whose root divides `a`.
 * @param[out] flags Receives what `sqrt` and `div` raise, and the zero-divisor flags, ORed in.
 * @return The result; the largest number with a's sign when `b` is zero.
 */
constexpr u32 rsqrt(u32 a, u32 b, u32& flags) {
    // A zero root divides nothing: the largest number with a's sign (documented).
    if (is_zero(b)) {
        // 0 / 0 is invalid; any other number over zero is a divide by zero.
        flags |= is_zero(a) ? kInvalid : kDivideByZero;
        return (a & kSign) | kMax;
    }

    return div(a, sqrt(b, flags), flags);
}

/**
 * Returns a key that orders patterns as sign and magnitude, with every zero equal.
 *
 * @param x A pattern.
 * @return A signed key: patterns compare as numbers when their keys do.
 */
constexpr s64 key(u32 x) {
    // Every zero, whatever its sign, sorts as 0.
    if (is_zero(x)) {
        return 0;
    }

    // A negative pattern sorts below every positive one, larger magnitudes further down.
    return (x & kSign) ? -static_cast<s64>(x & kMax) : static_cast<s64>(x);
}

/**
 * Returns the larger of two numbers.
 *
 * @param a The first number.
 * @param b The second number.
 * @return `b` when its key is larger, otherwise `a`.
 */
constexpr u32 max(u32 a, u32 b) {
    // On a tie, including two zeros of different sign, `a` is returned.
    return key(b) > key(a) ? b : a;
}

/**
 * Returns the smaller of two numbers.
 *
 * @param a The first number.
 * @param b The second number.
 * @return `b` when its key is smaller, otherwise `a`.
 */
constexpr u32 min(u32 a, u32 b) {
    // On a tie, including two zeros of different sign, `a` is returned.
    return key(b) < key(a) ? b : a;
}

/**
 * Returns an integer divided by 2^shift.
 *
 * @param value The integer.
 * @param shift The power of two to divide by.
 * @return The number, cut to 24 bits.
 */
constexpr u32 from_int(s32 value, s32 shift = 0) {
    // An integer cannot leave the range, so the flags of `pack` are not kept.
    u32 unused = 0;

    // Sign and magnitude, with the magnitude taken in 64 bits so the most negative value is safe.
    u64 magnitude =
        value < 0 ? static_cast<u64>(-static_cast<s64>(value)) : static_cast<u64>(value);

    // Unit 150 is the bias 127 plus the 23 fraction bits: a magnitude counts as itself there.
    return pack(value < 0 ? kSign : 0, magnitude, 150 - shift, unused);
}

/**
 * Returns the number times 2^shift as an integer, cut towards zero, saturating.
 *
 * @param x The number.
 * @param shift The power of two to multiply by.
 * @return The integer; 0 for a zero or a size below 1, and the most negative or most positive
 *         32-bit value when it does not fit.
 */
constexpr s32 to_int(u32 x, s32 shift = 0) {
    // A zero is the integer 0.
    if (is_zero(x)) {
        return 0;
    }

    // 150 is the bias 127 plus the 23 fraction bits, so the significand counts as an integer.
    s32 e = exponent(x) - 150 + shift;  // the mantissa's last place is worth 2^e

    // The significand is at least 2^23, so e of 8 or more means 2^31 or more: saturate.
    if (e >= 8) {
        return (x & kSign) ? static_cast<s32>(0x80000000u) : 0x7FFFFFFF;
    }

    // The significand is below 2^24, so shifting it right by 24 or more leaves nothing.
    if (e <= -24) {
        return 0;
    }

    // Shift to the integer scale; a right shift drops the low bits, which cuts towards zero.
    s64 magnitude = static_cast<s64>(e >= 0 ? mantissa(x) << e : mantissa(x) >> -e);

    // The magnitude is below 2^31 here, so the narrowing keeps it whole.
    return static_cast<s32>((x & kSign) ? -magnitude : magnitude);
}

/**
 * Converts a pattern to the host's double, exactly.
 *
 * To and from the host's numbers, for the few functions (sines, arctangents) that are computed
 * there.
 *
 * @param x A pattern.
 * @return Its value as a double, exactly.
 */
inline double to_double(u32 x) {
    // A zero keeps its sign.
    if (is_zero(x)) {
        return (x & kSign) ? -0.0 : 0.0;
    }

    // 150 is the bias 127 plus the 23 fraction bits, so the significand counts as an integer.
    double v = std::ldexp(static_cast<double>(mantissa(x)), exponent(x) - 150);
    return (x & kSign) ? -v : v;
}

/**
 * Converts a host double to a pattern, the counterpart of `to_double`.
 *
 * @param v The value.
 * @param[out] flags Receives kOverflow, ORed in, for an infinity and what `pack` raises.
 * @return The pattern, cut to 24 bits; zero for a NaN.
 */
inline u32 from_double(double v, u32& flags) {
    // A zero keeps its sign; a NaN, which the console has no pattern for, becomes a zero.
    if (v == 0.0 || std::isnan(v)) {
        return std::signbit(v) ? kSign : 0;
    }

    u32 sign = std::signbit(v) ? kSign : 0;

    // An infinity is a result too large: the largest number with its sign (documented).
    if (std::isinf(v)) {
        flags |= kOverflow;
        return sign | kMax;
    }

    int e = 0;
    double m = std::frexp(std::fabs(v), &e);  // m in [0.5, 1)

    // m is scaled to a 40-bit magnitude, so the unit is e - 40 + 150 = e + 110.
    return pack(sign, static_cast<u64>(std::ldexp(m, 40)), e + 110, flags);
}

}  // namespace ps2::fp
