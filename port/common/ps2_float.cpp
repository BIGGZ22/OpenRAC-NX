// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tfrag_light.rs
// (module ps2) and moby_anim.rs (ftoi): ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The PS2's float arithmetic on bit patterns (ps2_float.h, which says what is modelled).

#include "common/ps2_float.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace openrac::ps2 {

namespace {

u64 isqrt(u64 n) {
    auto r = static_cast<u64>(std::sqrt(static_cast<double>(n)));
    while (r * r > n) {
        --r;
    }
    while ((r + 1) * (r + 1) <= n) {
        ++r;
    }
    return r;
}

s64 order_key(u32 x) {
    return (x & kSign) != 0 ? -static_cast<s64>(x & kMax) : static_cast<s64>(x);
}

using detail::exponent;
using detail::mantissa;
using detail::pack;

}  // namespace

u32 div(u32 a, u32 b) {
    const u32 sign = (a ^ b) & kSign;
    if (exponent(b) == 0) {
        return sign | kMax;
    }
    if (exponent(a) == 0) {
        return sign;
    }
    const u64 q = (mantissa(a) << 24) / mantissa(b);  // in [2^23, 2^25)
    return pack(sign, q, exponent(a) - exponent(b) + 127 - 1);
}

u32 sqrt(u32 x) {
    if (exponent(x) == 0) {
        return 0;
    }
    const int e = exponent(x) - 127;
    const int odd = ((e % 2) + 2) % 2;
    const u64 r = isqrt(mantissa(x) << (23 + odd));  // in [2^23, 2^24)
    return pack(0, r, (e - odd) / 2 + 127);
}

u32 itof12(s32 i) {
    if (i == 0) {
        return 0;
    }
    const u64 magnitude =
        i < 0 ? u64{0} - static_cast<u64>(static_cast<s64>(i)) : static_cast<u64>(i);
    return pack(i < 0 ? kSign : 0, magnitude, 127 + 23 - 12);
}

u32 itof0(s32 i) {
    if (i == 0) {
        return 0;
    }
    const u64 magnitude =
        i < 0 ? u64{0} - static_cast<u64>(static_cast<s64>(i)) : static_cast<u64>(i);
    return pack(i < 0 ? kSign : 0, magnitude, 127 + 23);
}

f32 to_host(u32 x) {
    if (exponent(x) == 0xff) {
        return (x & kSign) != 0 ? -std::numeric_limits<f32>::max()
                                : std::numeric_limits<f32>::max();
    }
    return std::bit_cast<f32>(x);
}

u32 max(u32 a, u32 b) {
    return order_key(b) > order_key(a) ? b : a;
}

u32 min(u32 a, u32 b) {
    return order_key(b) < order_key(a) ? b : a;
}

s32 ftoi(u32 x, int n) {
    const int e = exponent(x);
    if (e == 0) {
        return 0;
    }
    const bool negative = (x & kSign) != 0;
    if (e == 255) {
        return negative ? std::numeric_limits<s32>::min() : std::numeric_limits<s32>::max();
    }
    double v = std::trunc(static_cast<double>(to_float(x & ~kSign)) * std::ldexp(1.0, n));
    if (negative) {
        v = -v;
    }
    v = std::clamp(
        v,
        static_cast<double>(std::numeric_limits<s32>::min()),
        static_cast<double>(std::numeric_limits<s32>::max())
    );
    return static_cast<s32>(v);
}

}  // namespace openrac::ps2
