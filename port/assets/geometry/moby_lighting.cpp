// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The moby lighting model (moby_lighting.h), done with the console's vector-unit arithmetic so
// that the colours come out bit for bit as the game computes them: every add, multiply and
// reciprocal square root goes through the ps2:: helpers on float bit patterns. The file builds a
// rotation from three angles (with the unit's sine polynomial), turns a light set into model-space
// lights, and then shades each vertex normal into an RGBA colour, packet by packet.

#include "assets/geometry/moby_lighting.h"

#include <algorithm>

namespace openrac::assets::rac1 {

namespace {

using ps2::V4;

// Float bit patterns of the constants the sine routine loads (1.5707963, pi, -pi).
constexpr u32 kHalfPi = 0x3fc9'0fda;  // 1.57079625, the routine's first constant
constexpr u32 kPi = 0x4049'0fda;
constexpr u32 kNegPi = 0xc049'0fda;
// The sine polynomial's coefficients of x^3, x^5, x^7, x^9.
constexpr std::array<u32, 4> kSinCoefficients =
    {0xbe2a'aaa4, 0x3c08'873e, 0xb94f'b21d, 0x362e'9c14};

// mulax ACC, a, v.x; madday ACC, b, v.y; maddz r, c, v.z: (a v.x + b v.y) + c v.z
// per lane.
V4 mat3(const V4& a, const V4& b, const V4& c, const std::array<u32, 3>& v) {
    V4 r{};

    // The same multiply-add is done on each of the four lanes.
    for (std::size_t k = 0; k < 4; ++k) {
        r[k] = ps2::add(ps2::add(ps2::mul(a[k], v[0]), ps2::mul(b[k], v[1])), ps2::mul(c[k], v[2]));
    }

    return r;
}

// Whether a float bit pattern is zero or denormal (exponent bits all clear), which the unit
// treats as zero.
bool is_zero(u32 x) {
    return (x & 0x7f80'0000) == 0;
}

// The first three lanes of a vector.
std::array<u32, 3> xyz(const V4& v) {
    return {v[0], v[1], v[2]};
}

}  // namespace

// The sine and cosine of an angle (float bits), as the unit's routine computes them: the cosine
// is the sine of the angle plus a quarter turn, and each argument is folded into -pi/2..pi/2
// before the polynomial.
std::pair<u32, u32> vu0_sin_cos(u32 a) {
    const u32 b = ps2::add(a, kHalfPi);
    const u32 pi = ps2::add(0, kPi);
    const u32 npi = ps2::add(0, kNegPi);
    auto fold = [&](u32 x) {
        return ps2::max(ps2::min(x, ps2::sub(pi, x)), ps2::sub(npi, x));
    };
    auto poly = [](u32 x) {
        const u32 x2 = ps2::mul(x, x);
        const u32 x3 = ps2::mul(x, x2);
        const u32 x5 = ps2::mul(x3, x2);
        const u32 x7 = ps2::mul(x5, x2);
        const u32 x9 = ps2::mul(x7, x2);
        u32 acc = ps2::mul(x, ps2::kOne);
        acc = ps2::add(acc, ps2::mul(x3, kSinCoefficients[0]));
        acc = ps2::add(acc, ps2::mul(x5, kSinCoefficients[1]));
        acc = ps2::add(acc, ps2::mul(x7, kSinCoefficients[2]));
        return ps2::add(acc, ps2::mul(x9, kSinCoefficients[3]));
    };
    return {poly(fold(a)), poly(fold(b))};
}

// The rotation matrix rows for three angles (X, Y, Z), applied in that order, starting from the
// identity.
Rows rotation_rows(const std::array<f32, 3>& angles) {
    const u32 one = ps2::kOne;
    const std::array<u32, 3> r = {ps2::bits(angles[0]), ps2::bits(angles[1]), ps2::bits(angles[2])};
    // The routine adds the angles to zero first; that add's zero flags decide
    // which steps run.
    const std::array<u32, 3> a = {ps2::add(r[0], 0), ps2::add(r[1], 0), ps2::add(r[2], 0)};
    Rows rows = kIdentityRows;
    // rows' = y0 row.x + y1 row.y + y2 row.z, from the rows before the step.
    auto apply = [&](const V4& y0, const V4& y1, const V4& y2) {
        // Each row of the matrix is transformed by the step's three rows.
        for (V4& row : rows) {
            row = mat3(y0, y1, y2, xyz(row));
        }
    };

    // A zero angle skips its rotation step, because the routine tests the angle's zero flag.
    if (!is_zero(a[0])) {
        const auto [s, c] = vu0_sin_cos(r[0]);
        rows[1][1] = ps2::add(0, c);
        rows[1][2] = ps2::add(0, s);
        rows[2][2] = ps2::add(0, c);
        rows[2][1] = ps2::sub(0, s);
    }

    // The Y rotation runs only for a non-zero angle, as above.
    if (!is_zero(a[1])) {
        const auto [s, c] = vu0_sin_cos(ps2::add(0, a[1]));
        apply(
            {ps2::add(0, c), 0, ps2::sub(0, s), 0},
            {0, one, 0, 0},
            {ps2::add(0, s), 0, ps2::add(0, c), 0}
        );
    }

    // The Z rotation runs only for a non-zero angle, as above.
    if (!is_zero(a[2])) {
        const auto [s, c] = vu0_sin_cos(ps2::add(0, a[2]));
        apply(
            {ps2::add(0, c), ps2::add(0, s), 0, 0},
            {ps2::sub(0, s), ps2::add(0, c), 0, 0},
            {0, 0, one, 0}
        );
    }

    return rows;
}

// The rows for a moby instance: the rotation, with the second row negated when bit 15 of `mode` is
// set.
Rows instance_rows(const std::array<f32, 3>& angles, u16 mode) {
    Rows rows = rotation_rows(angles);

    // Bit 15 of the mode word negates the second row.
    if (mode & 0x8000) {
        // Negate the first three lanes of the row; the fourth is not part of the rotation.
        for (std::size_t k = 0; k < 3; ++k) {
            rows[1][k] = ps2::sub(0, rows[1][k]);
        }
    }

    return rows;
}

// The lights of one moby for a given orientation: the two directional lights of the set named by
// `light_word` (blended with a second set when its fade byte is non-zero), moved into model space,
// plus the ambient colour and alpha.
MobyLights moby_lights(
    const Rows& rows,
    const LightBank& bank,
    u32 light_word,
    const std::array<u8, 3>& ambient,
    u8 alpha
) {
    // (-r0.c, -r1.c, -r2.c) for c = x, y, z (0x211d88..0x211da8).
    auto neg = [&](std::size_t c) -> V4 {
        return {ps2::sub(0, rows[0][c]), ps2::sub(0, rows[1][c]), ps2::sub(0, rows[2][c]), 0};
    };
    const V4 n0 = neg(0);
    const V4 n1 = neg(1);
    const V4 n2 = neg(2);
    auto to_model = [&](const V4& d) {
        return mat3(n0, n1, n2, xyz(d));
    };
    const u8 i0 = static_cast<u8>(light_word);
    const u8 i1 = static_cast<u8>(light_word >> 8);
    const u8 fade = static_cast<u8>(light_word >> 16);
    // The set index reads a 16-set scratchpad copy; past it lie the point
    // lights, which a load-time pass does not have: a zero set, as the game's
    // unused sets are.
    auto set = [&](u8 i) {
        return i < bank.sets.size() ? bank.sets[i] : DirLightSet{};
    };
    V4 la{};
    V4 lb{};
    V4 ca{};
    V4 cb{};

    // A fade of 0 means a single light set, used as it is.
    if (fade == 0) {
        const DirLightSet s = set(i0);
        la = to_model(ps2::bits(s.dir_a));
        lb = to_model(ps2::bits(s.dir_b));
        ca = ps2::bits(s.color_a);
        cb = ps2::bits(s.color_b);
    } else {
        // A non-zero fade blends two light sets by weights t and w.
        // 0x212000: t = itof12(fade << 4), w = itof12(0x1000 - (fade << 4));
        // directions (xyz) and colours (xyzw) blended, then L = (-R^T d) *
        // rsqrt(|d|^2), |d|^2 = (x^2 + y^2) + 1 * z^2.
        const u32 t = ps2::itof12(static_cast<s32>(fade) << 4);
        const u32 w = ps2::itof12(0x1000 - (static_cast<s32>(fade) << 4));
        const DirLightSet s0 = set(i0);
        const DirLightSet s1 = set(i1);
        // Lanes past n are not written: set 0's value stays.
        auto blend =
            [&](const std::array<f32, 4>& fa, const std::array<f32, 4>& fb, std::size_t n) {
                const V4 a = ps2::bits(fa);
                const V4 b = ps2::bits(fb);
                V4 r = a;

                // Blend only the first n lanes; the others keep the first set's values.
                for (std::size_t k = 0; k < n; ++k) {
                    r[k] = ps2::add(ps2::mul(a[k], w), ps2::mul(b[k], t));
                }

                return r;
            };
        auto direction = [&](const V4& d) {
            const u32 q = ps2::rsqrt(
                ps2::kOne,
                ps2::add(
                    ps2::add(ps2::mul(d[0], d[0]), ps2::mul(d[1], d[1])),
                    ps2::mul(ps2::kOne, ps2::mul(d[2], d[2]))
                )
            );
            const V4 l = to_model(d);
            return V4{ps2::mul(l[0], q), ps2::mul(l[1], q), ps2::mul(l[2], q), l[3]};
        };
        la = direction(blend(s0.dir_a, s1.dir_a, 3));
        lb = direction(blend(s0.dir_b, s1.dir_b, 3));
        ca = blend(s0.color_a, s1.color_a, 4);
        cb = blend(s0.color_b, s1.color_b, 4);
    }

    MobyLights out;
    // Lane x = light 0, y = light 1, z = light 2 (the point light: 0 here).
    out.rows = {V4{la[0], lb[0], 0, 0}, V4{la[1], lb[1], 0, 0}, V4{la[2], lb[2], 0, 0}};
    const V4 k = {ca[3], cb[3], 0, 0};

    // Turn the two per-light factors into the negated clamp factors the shading uses (lane 3 is
    // one).
    for (std::size_t i = 0; i < 4; ++i) {
        out.neg_k[i] = ps2::sub(i == 3 ? ps2::kOne : 0, k[i] & ~ps2::kSign);
    }

    out.colors = {V4{ca[0], ca[1], ca[2], 0}, V4{cb[0], cb[1], cb[2], 0}, V4{}};
    const std::array<u8, 4> amb = {ambient[0], ambient[1], ambient[2], alpha};

    // The ambient lanes are float bit patterns with the byte as the mantissa, as the routine builds
    // them.
    for (std::size_t i = 0; i < 4; ++i) {
        out.ambient[i] = 0x4780'0000u | amb[i];
    }

    return out;
}

// Converts the lit colour (16-bit lanes of the unit's fixed point) to bytes, scaled by the
// vertex's multiplier and clamped to the signed 16-bit range first.
std::array<u8, 4> pack_colour(const V4& colour, const std::array<u8, 4>& multiplier) {
    std::array<u8, 4> out{};

    // Pack the four channels.
    for (std::size_t k = 0; k < 4; ++k) {
        const s32 h = static_cast<s16>(static_cast<u16>(colour[k]));
        const s32 p = std::clamp(h * multiplier[k], -32768, 32767);
        out[k] = static_cast<u8>(p >> 7);
    }

    return out;
}

// The colour of one vertex: its normal (from azimuth and elevation table entries) is rotated by
// `m`, normalised, and dotted with the lights, then combined with the ambient term.
std::array<u8, 4> light_vertex(
    const MobyLights& l,
    const NormalTable& table,
    const Rows& m,
    u8 azimuth,
    u8 elevation,
    const std::array<u8, 4>& multiplier
) {
    const auto [ca, sa] = table.entries[azimuth];
    const auto [ce, se] = table.entries[elevation];
    // (cos a cos e, sin a cos e, sin e): the normal, not normalised.
    const std::array<u32, 3> n = {ps2::mul(ca, ce), ps2::mul(sa, ce), se};
    const V4 np = mat3(m[0], m[1], m[2], n);
    const u32 q = ps2::rsqrt(
        ps2::kOne,
        ps2::add(ps2::add(ps2::mul(np[0], np[0]), ps2::mul(np[1], np[1])), ps2::mul(np[2], np[2]))
    );
    // d_k = L_k . n'; f = max(d, d * -|K|).
    const V4 d = mat3(l.rows[0], l.rows[1], l.rows[2], xyz(np));
    std::array<u32, 3> f{};

    // One diffuse term per light.
    for (std::size_t k = 0; k < 3; ++k) {
        f[k] = ps2::max(d[k], ps2::mul(d[k], l.neg_k[k]));
    }

    // s = C0 f.x + C1 f.y + C2 f.z; c = s * Q + ambient * 1.
    const V4 s = mat3(l.colors[0], l.colors[1], l.colors[2], f);
    V4 c{};

    // Combine the three weighted colours with the ambient term, per channel.
    for (std::size_t k = 0; k < 4; ++k) {
        c[k] = ps2::add(ps2::mul(s[k], q), ps2::mul(l.ambient[k], ps2::kOne));
    }

    return pack_colour(c, multiplier);
}

// The colours of every vertex of every packet of one level of detail, in packet order. A packet
// ends with vertices duplicated from other packets, which take the colour already computed for
// their id (the cache is shared across the packets).
std::vector<std::vector<std::array<u8, 4>>> light_lod(
    std::span<const MobyPacket> packets, const MobyLights& lights, const NormalTable& table
) {
    std::array<std::array<u8, 4>, 512> cache{};
    std::vector<std::vector<std::array<u8, 4>>> out;

    // Light every packet of the level of detail on its own.
    for (const MobyPacket& p : packets) {
        const auto multipliers = p.rgba_multiplier_records();
        std::vector<std::array<u8, 4>> colours;
        colours.reserve(p.vertices.size());
        std::size_t in_file = 0;

        // Find the first duplicated vertex: those before it are the packet's own, the rest copy
        // another packet's.
        while (in_file < p.vertices.size() && !p.vertices[in_file].duplicate) {
            ++in_file;
        }

        // Light the packet's own vertices; a vertex without a multiplier record uses 0x80, the
        // neutral value.
        for (std::size_t i = 0; i < in_file; ++i) {
            const MobyVertex& v = p.vertices[i];
            const std::array<u8, 4> mult =
                i < multipliers.size() ? multipliers[i] : std::array<u8, 4>{0x80, 0x80, 0x80, 0x80};
            colours.push_back(light_vertex(
                lights, table, kIdentityRows, v.normal_azimuth, v.normal_elevation, mult
            ));
        }

        // Remember each lit vertex by id, so that later duplicates can take its colour.
        for (std::size_t i = 0; i < in_file; ++i) {
            cache[p.vertices[i].id & 0x1ff] = colours[i];
        }

        // A duplicate vertex takes the colour that the vertex with its id was given.
        for (std::size_t i = in_file; i < p.vertices.size(); ++i) {
            colours.push_back(cache[p.vertices[i].id & 0x1ff]);
        }

        out.push_back(std::move(colours));
    }

    return out;
}

}  // namespace openrac::assets::rac1
