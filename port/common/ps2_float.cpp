// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tfrag_light.rs
// (module ps2) and moby_anim.rs (ftoi): ISC License, Copyright (c) 2026 ReRAC contributors.
//
// PS2 float arithmetic on bit patterns; see ps2_float.h for what is modelled.

#include "common/ps2_float.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace openrac::ps2 {

namespace {

int exponent(u32 x) {
    return static_cast<int>((x >> 23) & 0xff);
}

u64 mantissa(u32 x) {
    return (x & 0x7f'ffff) | 0x80'0000;
}

// Packs sign * mag * 2^lsb_exp (mag > 0; lsb_exp is the biased exponent of
// mag's bit 23), truncating to 24 significant bits.
u32 pack(u32 sign, u64 mag, int lsb_exp) {
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

}  // namespace

u32 mul(u32 a, u32 b) {
    const u32 sign = (a ^ b) & kSign;
    if (exponent(a) == 0 || exponent(b) == 0) {
        return sign;
    }
    // The 48-bit product has its binary point at bit 46; bit 23 of (p >> 23)
    // is worth 2^(ea + eb - 254).
    return pack(sign, (mantissa(a) * mantissa(b)) >> 23, exponent(a) + exponent(b) - 127);
}

u32 add(u32 a, u32 b) {
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
    return pack(sum < 0 ? kSign : 0, magnitude, exponent(lo));
}

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
