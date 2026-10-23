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

// Returns a * b as the multiplier gives it: the product truncated toward zero.
u32 mul(u32 a, u32 b);

// Returns a + b as the adder gives it: the smaller operand's bits more than one place below the
// larger one's last bit dropped first, then the sum truncated toward zero.
u32 add(u32 a, u32 b);

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
