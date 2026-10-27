// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tfrag_light.rs
// (module ps2) and moby_anim.rs (ftoi): ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The PS2's FPU and vector-unit float arithmetic on raw bit patterns. The
// game's baked lighting and its animation evaluator run on these units, and
// their results end up as bytes (colour channels on a 1/128 grid, vertex
// positions after ftoi): to reproduce those bytes the arithmetic has to be the
// console's, not IEEE's.
//
// Modelled (ReRAC docs/plan/tfrag_lighting.md): no NaN, infinity or denormal
// (exponent 0 is zero, exponent 255 an ordinary value, overflow clamps to
// +-0x7fffffff, underflow flushes to +-0); every result rounds toward zero; the
// adder first drops the bits of the smaller operand that lie more than one bit
// below the larger one's last bit, and returns the larger operand unchanged
// when the exponents differ by 25 or more. Not modelled: the multiplier's rare
// last-bit deviations from a truncated exact product.

#pragma once

#include <array>
#include <bit>
#include <cstdint>

namespace openrac::ps2 {

// The bit-pattern types, spelled as the asset readers spell them (assets/bytes.h, a layer above).
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using s32 = std::int32_t;
using s64 = std::int64_t;
using f32 = float;

// Bit patterns: the sign bit, the largest magnitude (what an overflow clamps to), 1.0 and -1.0.
constexpr u32 kSign = 0x8000'0000;
constexpr u32 kMax = 0x7fff'ffff;
constexpr u32 kOne = 0x3f80'0000;
constexpr u32 kNegOne = 0xbf80'0000;

// Four vector lanes as raw float bits.
using V4 = std::array<u32, 4>;

// Returns a host float's bit pattern, unchanged.
inline u32 bits(f32 x) {
    return std::bit_cast<u32>(x);
}

// Returns the host float with this bit pattern, unchanged: an exponent of 255 reads as the host's
// infinity or NaN, where to_host() gives the value the console means.
inline f32 to_float(u32 x) {
    return std::bit_cast<f32>(x);
}

// Returns the host float a value means: exponent 255 is an ordinary huge number on the console, not
// an infinity or NaN, so it becomes the largest float of its sign.
f32 to_host(u32 x);

// Returns four host floats' bit patterns, as bits() does for one.
inline V4 bits(const std::array<f32, 4>& v) {
    return {bits(v[0]), bits(v[1]), bits(v[2]), bits(v[3])};
}

// Returns the four host floats with these bit patterns, as to_float() does for one.
inline std::array<f32, 4> to_floats(const V4& v) {
    return {to_float(v[0]), to_float(v[1]), to_float(v[2]), to_float(v[3])};
}

// The pieces mul() and add() share, defined here so the hot callers (the animation evaluator, the
// lighting) can inline them.
namespace detail {

inline int exponent(u32 x) {
    return static_cast<int>((x >> 23) & 0xff);
}

inline u64 mantissa(u32 x) {
    return (x & 0x7f'ffff) | 0x80'0000;
}

// Packs sign * mag * 2^lsb_exp (mag > 0; lsb_exp is the biased exponent of
// mag's bit 23), truncating to 24 significant bits.
inline u32 pack(u32 sign, u64 mag, int lsb_exp) {
    const int k = 63 - std::countl_zero(mag);
    const int e = lsb_exp + (k - 23);
    const u64 m = k >= 23 ? mag >> (k - 23) : mag << (23 - k);
    if (e > 255) {
        return sign | kMax;
    }
    if (e < 1) {
        return sign;
    }
    return sign | static_cast<u32>(e) << 23 | (static_cast<u32>(m) & 0x7f'ffff);
}

}  // namespace detail

// Returns a * b as the multiplier gives it: the product truncated toward zero.
inline u32 mul(u32 a, u32 b) {
    using detail::exponent, detail::mantissa;
    const u32 sign = (a ^ b) & kSign;
    if (exponent(a) == 0 || exponent(b) == 0) {
        return sign;
    }
    // The 48-bit product has its binary point at bit 46; bit 23 of (p >> 23)
    // is worth 2^(ea + eb - 254).
    return detail::pack(sign, (mantissa(a) * mantissa(b)) >> 23, exponent(a) + exponent(b) - 127);
}

// Returns a + b as the adder gives it: the smaller operand's bits more than one place below the
// larger one's last bit dropped first, then the sum truncated toward zero.
inline u32 add(u32 a, u32 b) {
    using detail::exponent, detail::mantissa;
    const int ea = exponent(a);
    const int eb = exponent(b);
    if (ea == 0 || eb == 0) {
        if (ea != 0) {
            return a;
        }
        if (eb != 0) {
            return b;
        }
        return (a & b & kSign) != 0 ? kSign : 0;
    }
    const u32 hi = ea >= eb ? a : b;
    u32 lo = ea >= eb ? b : a;
    const int d = exponent(hi) - exponent(lo);
    if (d >= 25) {
        return hi;
    }
    if (d >= 1) {
        lo &= ~u32{0} << (d - 1);
    }
    auto signed_value = [](u32 x, u64 m) {
        return (x & kSign) != 0 ? -static_cast<s64>(m) : static_cast<s64>(m);
    };
    const s64 sum = signed_value(hi, mantissa(hi) << d) + signed_value(lo, mantissa(lo));
    if (sum == 0) {
        return 0;
    }
    const u64 magnitude = sum < 0 ? static_cast<u64>(-sum) : static_cast<u64>(sum);
    return detail::pack(sum < 0 ? kSign : 0, magnitude, exponent(lo));
}

// Returns a - b: the sum with b's sign turned, as the adder forms it.
inline u32 sub(u32 a, u32 b) {
    return add(a, b ^ kSign);
}

// Returns a / b truncated (DIV); a division by zero gives +-kMax.
u32 div(u32 a, u32 b);

// Returns the square root of |x|, truncated (SQRT).
u32 sqrt(u32 x);

// Returns a / sqrt(b) (RSQRT): the root is truncated first, then divided.
inline u32 rsqrt(u32 a, u32 b) {
    return div(a, sqrt(b));
}

// Returns the signed integer `i` / 4096 as a float (ITOF12); integers wider than 24 bits truncate.
u32 itof12(s32 i);

// Returns the signed integer `i` as a float (ITOF0); integers wider than 24 bits truncate.
u32 itof0(s32 i);

// Returns the larger of a and b (MAX), comparing sign and magnitude as the units do.
u32 max(u32 a, u32 b);

// Returns the smaller of a and b (MIN), comparing sign and magnitude as the units do.
u32 min(u32 a, u32 b);

// Returns x * 2^n truncated toward zero and saturated to the s32 range (the VU's ftoiN).
s32 ftoi(u32 x, int n);

// Returns the dot product of three lanes as the game's VU code forms it:
// vmul t, a, b; vadday.x ACC, t, t; vmaddz.x r, one, t, that is
// (ax*bx + ay*by) + 1.0*(az*bz).
inline u32 dot3(const V4& a, const V4& b) {
    return add(add(mul(a[0], b[0]), mul(a[1], b[1])), mul(kOne, mul(a[2], b[2])));
}

// Returns every lane of `a` times the scalar `s`.
inline V4 scale(const V4& a, u32 s) {
    return {mul(a[0], s), mul(a[1], s), mul(a[2], s), mul(a[3], s)};
}

// Returns the lanes of a and b added pairwise.
inline V4 add(const V4& a, const V4& b) {
    return {add(a[0], b[0]), add(a[1], b[1]), add(a[2], b[2]), add(a[3], b[3])};
}

}  // namespace openrac::ps2
