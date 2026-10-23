// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/shrub_light.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// LightShrubs on PS2 float arithmetic, and VU1 program 56467's memory map.

#include "assets/geometry/shrub_lighting.h"

#include <algorithm>

namespace openrac::assets::rac1 {

namespace {

using ps2::V4;

// vmul.xyz; vadday.x; vmaddz.x (|c|^2), vrsqrt Q, vf0w, vmulq.xyz.
std::array<V4, 3> unit_columns(const ShrubInstance& inst) {
    std::array<V4, 3> out{};

    for (std::size_t i = 0; i < 3; ++i) {
        const V4 c = ps2::bits(inst.matrix[i]);
        const u32 q = ps2::rsqrt(ps2::kOne, ps2::dot3(c, c));
        out[i] = {ps2::mul(c[0], q), ps2::mul(c[1], q), ps2::mul(c[2], q), c[3]};
    }

    return out;
}

}  // namespace

u32 shrub_packed_ambient(const ShrubInstance& inst) {
    const auto r = static_cast<u32>(inst.colour[0]);
    const auto g = static_cast<u32>(inst.colour[1]);
    const auto b = static_cast<u32>(inst.colour[2]);
    return (b << 16 | g << 8 | r) | 0x8000'0000u;
}

std::array<u32, 3> shrub_column_lengths(const ShrubInstance& inst) {
    std::array<u32, 3> out{};

    for (std::size_t i = 0; i < 3; ++i) {
        const V4 c = ps2::bits(inst.matrix[i]);
        out[i] = ps2::sqrt(ps2::dot3(c, c));
    }

    return out;
}

std::array<f32, 4> shrub_instance_centre(const ShrubClass& shrub, const ShrubInstance& inst) {
    std::array<V4, 4> m{};

    for (std::size_t i = 0; i < 4; ++i) {
        m[i] = ps2::bits(inst.matrix[i]);
    }

    const V4 b = ps2::bits(shrub.header.bsphere);
    V4 c{};

    for (std::size_t k = 0; k < 3; ++k) {
        c[k] = ps2::add(
            ps2::add(ps2::mul(m[0][k], b[0]), ps2::mul(m[1][k], b[1])), ps2::mul(m[2][k], b[2])
        );
    }

    const auto lengths = shrub_column_lengths(inst);
    const f32 longest =
        std::max({ps2::to_float(lengths[0]), ps2::to_float(lengths[1]), ps2::to_float(lengths[2])});
    c[3] = ps2::mul(b[3], ps2::bits(longest));
    c = ps2::scale(c, ps2::bits(shrub.header.scale));
    return ps2::to_floats(
        {ps2::add(c[0], m[3][0]), ps2::add(c[1], m[3][1]), ps2::add(c[2], m[3][2]), c[3]}
    );
}

ShrubLightRegs shrub_light_regs(
    const ShrubClass& shrub,
    const ShrubInstance& inst,
    const LightBank& bank,
    const std::optional<ShrubPointLights>& points
) {
    // Run-time record +0x1c = the low half of dir_lights.
    const auto select = static_cast<u16>(inst.dir_lights);
    auto set = [&](u32 i) -> const DirLightSet& {
        return bank.sets[i & 0xf];
    };
    V4 ca, da, cb, db;

    if ((select & 0xff00) == 0) {
        // lqc2 of the set's four quadwords: no renormalisation.
        const DirLightSet& s = set(select);
        ca = ps2::bits(s.color_a);
        da = ps2::bits(s.dir_a);
        cb = ps2::bits(s.color_b);
        db = ps2::bits(s.dir_b);
    } else {
        // vitof12.x t; vsubx.w w = 1 - t; set A * w and set B * t on xyz only
        // (vmulw.xyz / vmulx.xyz), then vadd.xyzw: the back factors add unscaled.
        const u32 t = ps2::itof12(static_cast<s32>((u32{select} >> 4) & 0xff0));
        const u32 w = ps2::sub(ps2::kOne, t);
        const DirLightSet& a = set(select);
        const DirLightSet& b = set(u32{select} >> 4);
        auto blend = [&](const std::array<f32, 4>& x4, const std::array<f32, 4>& y4) {
            const V4 x = ps2::bits(x4);
            const V4 y = ps2::bits(y4);
            return V4{
                ps2::add(ps2::mul(x[0], w), ps2::mul(y[0], t)),
                ps2::add(ps2::mul(x[1], w), ps2::mul(y[1], t)),
                ps2::add(ps2::mul(x[2], w), ps2::mul(y[2], t)),
                ps2::add(x[3], y[3]),
            };
        };
        ca = blend(a.color_a, b.color_a);
        cb = blend(a.color_b, b.color_b);
        da = blend(a.dir_a, b.dir_a);
        db = blend(a.dir_b, b.dir_b);

        for (V4* d : {&da, &db}) {
            const u32 q = ps2::rsqrt(ps2::kOne, ps2::dot3(*d, *d));
            *d = {ps2::mul((*d)[0], q), ps2::mul((*d)[1], q), ps2::mul((*d)[2], q), (*d)[3]};
        }
    }

    // vaddw.x / vaddw.y vf30, vf0, colour: the back factors; vsubw.w clears w.
    const u32 wa = ps2::add(0, ca[3]);
    const u32 wb = ps2::add(0, cb[3]);
    ca[3] = 0;
    cb[3] = 0;

    MergedPointLight point;

    if (points && points->bank != nullptr) {
        point = merge_point_lights(
            ps2::bits(shrub_instance_centre(shrub, inst)), *points->bank, points->list
        );
    }

    V4 dp = point.direction;
    V4 cp = point.color;
    const u32 wp = ps2::add(0, cp[3]);
    cp[3] = 0;
    dp[3] = 0;

    // vf4..vf6 = the rows of -N; L = vf4 * d.x + vf5 * d.y + vf6 * d.z = -N^T d.
    const auto n = unit_columns(inst);
    auto class_space = [&](const V4& d) {
        V4 l{};

        for (std::size_t c = 0; c < 3; ++c) {
            auto neg = [&](std::size_t k) {
                return ps2::sub(0, n[c][k]);
            };
            l[c] = ps2::add(
                ps2::add(ps2::mul(neg(0), d[0]), ps2::mul(neg(1), d[1])), ps2::mul(neg(2), d[2])
            );
        }

        return l;
    };
    const V4 la = class_space(da);
    const V4 lb = class_space(db);
    const V4 lp = class_space(dp);
    ShrubLightRegs regs;
    regs.lights.colors = {ca, cb, cp};
    regs.lights.back = {wa, wb, wp};

    // vaddx.x vf24, vf0, vf1 ...: each lane goes through vf0 + x.
    for (std::size_t i = 0; i < 3; ++i) {
        regs.lights.rows[i] = {ps2::add(0, la[i]), ps2::add(0, lb[i]), ps2::add(0, lp[i]), 0};
    }

    // lw col0.w; pextlb; pextlh; padduw 0x47800000.
    const u32 word = shrub_packed_ambient(inst);
    regs.ambient = color_floats(
        {static_cast<u8>(word),
         static_cast<u8>(word >> 8),
         static_cast<u8>(word >> 16),
         static_cast<u8>(word >> 24)}
    );
    return regs;
}

std::array<u8, 4> light_shrub_normal(const ShrubLightRegs& regs, const std::array<s16, 4>& normal) {
    return light_instance_normal(regs.lights, normal, regs.ambient, kInstanceColorClamp);
}

ShrubPalette light_shrub_instance(
    const ShrubClass& shrub,
    const ShrubInstance& inst,
    const LightBank& bank,
    const std::optional<ShrubPointLights>& points
) {
    const ShrubLightRegs regs = shrub_light_regs(shrub, inst, bank, points);
    ShrubPalette out{};

    for (std::size_t j = 0; j < kShrubNormals; ++j) {
        const std::array<s16, 4> n =
            j < shrub.normals.size() ? shrub.normals[j] : std::array<s16, 4>{};
        out[j] = light_shrub_normal(regs, n);
    }

    return out;
}

std::array<u8, 3> shrub_average_colour(const ShrubPalette& palette) {
    std::array<u32, 3> sum{};

    for (const auto& c : palette) {
        for (std::size_t k = 0; k < 3; ++k) {
            sum[k] += c[k];
        }
    }

    return {
        static_cast<u8>(sum[0] / kShrubNormals),
        static_cast<u8>(sum[1] / kShrubNormals),
        static_cast<u8>(sum[2] / kShrubNormals),
    };
}

u16 shrub_vu1_palette_base(u16 buffer, std::size_t k) {
    // iaddiu vi13, vi13, 5 after the matrix loads, then + 0x1c per instance.
    return static_cast<u16>(buffer + 5 + kShrubVu1SlotQwc * k);
}

u16 shrub_vu1_colour_address(
    std::size_t written_vertices, std::size_t vertex, u8 normal, u16 palette_base
) {
    const u32 extra = written_vertices == 6 && vertex == 3 ? palette_base : 0u;
    return static_cast<u16>((u32{normal} + palette_base + extra) % kShrubVu1Qwc);
}

ShrubVu1Data shrub_vu1_data(u16 address) {
    const auto a = static_cast<u16>(address % kShrubVu1Qwc);

    for (const u16 buffer : kShrubVu1InstanceBuffers) {
        const auto rel = static_cast<u16>(a - buffer);

        if (rel >= 1 && rel <= kShrubVu1SlotQwc * kShrubVu1Batch) {
            const std::size_t slot = (rel - 1u) / kShrubVu1SlotQwc;
            const auto qw = static_cast<u16>((rel - 1u) % kShrubVu1SlotQwc);

            if (qw < 4) {
                return {ShrubVu1Data::Kind::Matrix, buffer, slot, qw};
            }

            return {ShrubVu1Data::Kind::Palette, buffer, slot, static_cast<u16>(qw - 4)};
        }
    }

    return {ShrubVu1Data::Kind::Other, 0, 0, a};
}

}  // namespace openrac::assets::rac1
