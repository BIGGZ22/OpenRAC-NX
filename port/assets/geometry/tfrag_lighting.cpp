// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tfrag_light.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// LightTfrags on PS2 float arithmetic. The comments name the VU0 macro
// instructions each step mirrors.

#include "assets/geometry/tfrag_lighting.h"

namespace openrac::assets::rac1 {

namespace {

using ps2::V4;

// `vmadd` per lane: acc + a * s.
V4 madd(const V4& acc, const V4& a, u32 s) {
    return {
        ps2::add(acc[0], ps2::mul(a[0], s)),
        ps2::add(acc[1], ps2::mul(a[1], s)),
        ps2::add(acc[2], ps2::mul(a[2], s)),
        ps2::add(acc[3], ps2::mul(a[3], s)),
    };
}

// A colour byte as the float 65536 + c / 128 (pextlb / pextlh to words,
// padduw with 0x47800000).
V4 color_float(const std::array<u8, 4>& c) {
    return {0x4780'0000u + c[0], 0x4780'0000u + c[1], 0x4780'0000u + c[2], 0x4780'0000u + c[3]};
}

// vminibcx.xyz with 0x478000ff, then ppach / ppacb: the low byte of each lane.
std::array<u8, 4> pack_color(const V4& v) {
    constexpr u32 kClamp = 0x4780'00ff;
    return {
        static_cast<u8>(ps2::min(v[0], kClamp)),
        static_cast<u8>(ps2::min(v[1], kClamp)),
        static_cast<u8>(ps2::min(v[2], kClamp)),
        static_cast<u8>(v[3]),
    };
}

// The light set of one vertex as the VU registers hold it, with the back-face
// factors taken out and the colour w lanes cleared.
struct ActiveSet {
    V4 ca, da, cb, db;
    u32 wa, wb;
};

ActiveSet active_set(const LightBank& bank, u16 select) {
    auto set = [&](u32 i) -> const DirLightSet& {
        return bank.sets[i & 0xf];
    };
    ActiveSet s{};

    if ((select & 0xff00) == 0) {
        const DirLightSet& d = set(select);
        s.ca = ps2::bits(d.color_a);
        s.da = ps2::bits(d.dir_a);
        s.cb = ps2::bits(d.color_b);
        s.db = ps2::bits(d.dir_b);
    } else {
        // t = itof12((sel >> 4) & 0xff0) = hi / 256; w = 1 - t.
        const u32 t = ps2::itof12(static_cast<s32>((u32{select} >> 4) & 0xff0));
        const u32 w = ps2::sub(ps2::kOne, t);
        const DirLightSet& a = set(select);
        const DirLightSet& b = set(u32{select} >> 4);
        auto blend = [&](const std::array<f32, 4>& x, const std::array<f32, 4>& y) {
            return ps2::add(ps2::scale(ps2::bits(x), w), ps2::scale(ps2::bits(y), t));
        };
        auto normalise = [](const V4& v) {
            const u32 q = ps2::rsqrt(ps2::kOne, ps2::dot3(v, v));
            return V4{ps2::mul(v[0], q), ps2::mul(v[1], q), ps2::mul(v[2], q), v[3]};
        };
        s.ca = blend(a.color_a, b.color_a);
        s.cb = blend(a.color_b, b.color_b);
        s.da = normalise(blend(a.dir_a, b.dir_a));
        s.db = normalise(blend(a.dir_b, b.dir_b));
    }

    // vaddw.x / .y vf31 = 0 + colour.w; vsubw.w colour = 1 - 1.
    s.wa = ps2::add(0, s.ca[3]);
    s.wb = ps2::add(0, s.cb[3]);
    s.ca[3] = 0;
    s.cb[3] = 0;
    return s;
}

std::array<u8, 4> light_with_normal(const LightBank& bank, const V4& n, u16 color, u16 select) {
    const ActiveSet s = active_set(bank, select);
    u32 da = ps2::dot3(n, s.da);
    u32 db = ps2::dot3(n, s.db);
    da = ps2::max(da, ps2::mul(da, s.wa));
    db = ps2::max(db, ps2::mul(db, s.wb));
    V4 acc = ps2::scale(color_float(pext5(color)), ps2::kOne);
    acc = madd(acc, s.ca, da);
    return pack_color(madd(acc, s.cb, db));
}

}  // namespace

ps2::V4 tfrag_stored_normal(const NormalTable& table, u8 azimuth, u8 elevation) {
    // ld of table[az] and table[el]; vmulz.xy; vaddw.z; vsub.xyz vf0 - v.
    const auto [ca, sa] = table.entries[azimuth];
    const auto [ce, se] = table.entries[elevation];
    const u32 x = ps2::mul(ca, ce);
    const u32 y = ps2::mul(sa, ce);
    const u32 z = ps2::add(0, se);
    return {ps2::sub(0, x), ps2::sub(0, y), ps2::sub(0, z), se};
}

std::array<u8, 4> light_tfrag_vertex(
    const LightBank& bank, const NormalTable& table, const TfragLight& l
) {
    return light_with_normal(
        bank, tfrag_stored_normal(table, l.azimuth, l.elevation), l.color, l.light_select
    );
}

std::optional<std::array<u8, 4>> light_tfrag_point(
    const PointLight& light,
    const std::array<u32, 3>& base,
    const std::array<s16, 3>& local,
    const ps2::V4& n,
    const std::array<u8, 4>& rgba
) {
    const V4 c = ps2::bits(light.color);
    const V4 lp = ps2::bits(light.position);
    std::array<u32, 3> p{};

    for (std::size_t k = 0; k < 3; ++k) {
        p[k] = ps2::add(ps2::itof12(s32{local[k]} * 4), base[k]);
    }

    const u32 inv_r = ps2::div(ps2::kOne, lp[3]);
    const V4 dv = {ps2::sub(p[0], lp[0]), ps2::sub(p[1], lp[1]), ps2::sub(p[2], lp[2]), 0};
    const u32 dist2 = ps2::dot3(dv, dv);

    // vsubx.w vf0, r2, dist2 only sets flags; the sign of w skips the vertex.
    if ((ps2::sub(ps2::mul(lp[3], lp[3]), dist2) & ps2::kSign) != 0) {
        return std::nullopt;
    }

    const u32 dist = ps2::sqrt(dist2);
    const u32 falloff = ps2::sub(ps2::kOne, ps2::mul(inv_r, dist));
    const u32 scale = ps2::mul(falloff, ps2::div(ps2::kOne, dist));
    const V4 v = {ps2::mul(dv[0], scale), ps2::mul(dv[1], scale), ps2::mul(dv[2], scale), 0};
    u32 d = ps2::dot3(n, v);
    d = ps2::max(d, ps2::mul(d, c[3]));
    const V4 acc = ps2::scale(color_float(rgba), ps2::kOne);
    return pack_color(madd(acc, c, d));
}

std::vector<TfragRgba> light_tfrag(
    const Tfrag& t,
    const LightBank& bank,
    const NormalTable& table,
    const std::optional<TfragPointLights>& points
) {
    std::vector<TfragRgba> out = t.rgba;
    const std::size_t n = std::min({std::size_t{t.header.vert_count}, t.lights.size(), out.size()});
    std::vector<V4> normals(n);
    std::vector<std::array<u8, 4>> rgba(n);
    // Header 0x34 >= 0 selects one set for the whole tfrag (no RAC1 NTSC-U
    // level uses it: all are -1).
    const auto single = static_cast<s8>(t.header.dir_lights_one);

    for (std::size_t i = 0; i < n; ++i) {
        const TfragLight& l = t.lights[i];
        normals[i] = tfrag_stored_normal(table, l.azimuth, l.elevation);
        const u16 select = single >= 0 ? static_cast<u16>(single & 0xf) : l.light_select;
        rgba[i] = light_with_normal(bank, normals[i], l.color, select);
    }

    if (points && points->bank != nullptr) {
        std::array<u32, 3> base{};

        for (std::size_t k = 0; k < 3; ++k) {
            base[k] = ps2::itof12(static_cast<s32>(static_cast<u32>(points->origin[k]) << 2));
        }

        u32 list = u32{points->list} | 0xf'0000;

        while ((list & 0xf) != 0xf) {
            const PointLight& light = (*points->bank)[(list & 0xf) % kPointLightSlots];
            list >>= 4;

            for (std::size_t i = 0; i < n; ++i) {
                std::array<s16, 3> q{};

                if (i < t.positions.size()) {
                    q = {t.positions[i].x, t.positions[i].y, t.positions[i].z};
                }

                if (const auto lit = light_tfrag_point(light, base, q, normals[i], rgba[i])) {
                    rgba[i] = *lit;
                }
            }
        }
    }

    for (std::size_t i = 0; i < n; ++i) {
        out[i] = {rgba[i][0], rgba[i][1], rgba[i][2], rgba[i][3]};
    }

    return out;
}

}  // namespace openrac::assets::rac1
