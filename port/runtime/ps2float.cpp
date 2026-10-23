// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// openrac/ps2float.h: the C entry points to port/common/ps2_float.h's model of the EE's float
// arithmetic, for the functions hostgen translates with it (hostgen.json "ps2_float") and for the
// port's own C that computes as the console does. The games' library links it
// (cmake/Games.cmake).

#include "openrac/ps2float.h"

#include <bit>
#include <cstdint>

#include "common/ps2_float.h"

namespace {

namespace ps2 = openrac::ps2;

std::uint32_t bits(float x) {
    return std::bit_cast<std::uint32_t>(x);
}

float value(std::uint32_t x) {
    return std::bit_cast<float>(x);
}

// The order of the EE's comparisons: sign and magnitude, every zero (exponent 0) equal.
std::int64_t key(float x) {
    const std::uint32_t u = bits(x);
    if ((u & 0x7F800000u) == 0) {
        return 0;
    }
    const auto magnitude = static_cast<std::int64_t>(u & ps2::kMax);
    return (u & ps2::kSign) != 0 ? -magnitude : magnitude;
}

}  // namespace

extern "C" {

float openrac_ps2_add(float a, float b) {
    return value(ps2::add(bits(a), bits(b)));
}

float openrac_ps2_sub(float a, float b) {
    return value(ps2::sub(bits(a), bits(b)));
}

float openrac_ps2_mul(float a, float b) {
    return value(ps2::mul(bits(a), bits(b)));
}

float openrac_ps2_div(float a, float b) {
    return value(ps2::div(bits(a), bits(b)));
}

float openrac_ps2_sqrt(float x) {
    return value(ps2::sqrt(bits(x)));
}

float openrac_ps2_rsqrt(float a, float b) {
    return value(ps2::rsqrt(bits(a), bits(b)));
}

int openrac_ps2_lt(float a, float b) {
    return key(a) < key(b) ? 1 : 0;
}

int openrac_ps2_le(float a, float b) {
    return key(a) <= key(b) ? 1 : 0;
}

int openrac_ps2_gt(float a, float b) {
    return key(b) < key(a) ? 1 : 0;
}

int openrac_ps2_ge(float a, float b) {
    return key(b) <= key(a) ? 1 : 0;
}

int openrac_ps2_eq(float a, float b) {
    return key(a) == key(b) ? 1 : 0;
}

float openrac_ps2_itof(std::int32_t i) {
    return value(ps2::itof0(i));
}

std::int32_t openrac_ps2_ftoi(float x) {
    return ps2::ftoi(bits(x), 0);
}

}  // extern "C"
