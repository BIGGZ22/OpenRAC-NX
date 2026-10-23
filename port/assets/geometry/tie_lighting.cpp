// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tie_light.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// LightTies on PS2 float arithmetic.

#include "assets/geometry/tie_lighting.h"

#include <algorithm>

namespace openrac::assets::rac1 {

namespace {

using ps2::V4;

// The unit axis columns the loader leaves in the run-time matrix: column i's
// w lane is 1.0 / length (EE FPU div.s), and LightTies scales the column by it.
std::array<V4, 3> unit_columns(const TieInstance& inst) {
    const auto lengths = tie_column_lengths(inst);
    std::array<V4, 3> out{};

    for (std::size_t i = 0; i < 3; ++i) {
        const u32 inv = ps2::div(ps2::kOne, lengths[i]);
        const V4 c = ps2::bits(inst.matrix[i]);
        out[i] = {ps2::mul(c[0], inv), ps2::mul(c[1], inv), ps2::mul(c[2], inv), inv};
    }

    return out;
}

}  // namespace

std::array<u32, 3> tie_column_lengths(const TieInstance& inst) {
    std::array<u32, 3> out{};

    for (std::size_t i = 0; i < 3; ++i) {
        const V4 c = ps2::bits(inst.matrix[i]);
        out[i] = ps2::sqrt(ps2::dot3(c, c));
    }

    return out;
}

std::array<f32, 4> tie_instance_centre(const TieClass& tie, const TieInstance& inst) {
    std::array<V4, 4> m{};

    for (std::size_t i = 0; i < 4; ++i) {
        m[i] = ps2::bits(inst.matrix[i]);
    }

    const V4 b = ps2::bits(tie.header.bsphere);
    // vmulax / vmadday / vmaddaz with the three columns, vmaddw with vf0.
    V4 c{};

    for (std::size_t k = 0; k < 4; ++k) {
        const u32 acc = ps2::add(
            ps2::add(ps2::mul(m[0][k], b[0]), ps2::mul(m[1][k], b[1])), ps2::mul(m[2][k], b[2])
        );
        c[k] = ps2::add(acc, ps2::mul(k == 3 ? ps2::kOne : 0u, b[3]));
    }

    // The radius: bsphere.w * max(|c0|, |c1|, |c2|) (FPU mul.s; the max is a
    // float compare).
    const auto lengths = tie_column_lengths(inst);
    const f32 longest =
        std::max({ps2::to_float(lengths[0]), ps2::to_float(lengths[1]), ps2::to_float(lengths[2])});
    c[3] = ps2::mul(b[3], ps2::bits(longest));
    c = ps2::scale(c, ps2::bits(tie.header.scale));
    return ps2::to_floats(
        {ps2::add(c[0], m[3][0]), ps2::add(c[1], m[3][1]), ps2::add(c[2], m[3][2]), c[3]}
    );
}

InstanceLightRegs tie_light_regs(
    const TieClass& tie,
    const TieInstance& inst,
    const LightBank& bank,
    const std::optional<TiePointLights>& points
) {
    // Run-time record +0x1c = the low half of directional_lights.
    const auto select = static_cast<u16>(inst.directional_lights);
    auto set = [&](u32 i) -> const DirLightSet& {
        return bank.sets[i & 0xf];
    };
    V4 ca, da, cb, db;

    if ((select & 0xff00) == 0) {
        const DirLightSet& s = set(select);
        ca = ps2::bits(s.color_a);
        da = ps2::bits(s.dir_a);
        cb = ps2::bits(s.color_b);
        db = ps2::bits(s.dir_b);
    } else {
        // vitof12.x t = (sel >> 4) & 0xff0; vsubx.w w = 1 - t; set A (low
        // nibble) * w + set B (next nibble) * t, all four lanes.
        const u32 t = ps2::itof12(static_cast<s32>((u32{select} >> 4) & 0xff0));
        const u32 w = ps2::sub(ps2::kOne, t);
        const DirLightSet& a = set(select);
        const DirLightSet& b = set(u32{select} >> 4);
        auto blend = [&](const std::array<f32, 4>& x, const std::array<f32, 4>& y) {
            return ps2::add(ps2::scale(ps2::bits(x), w), ps2::scale(ps2::bits(y), t));
        };
        ca = blend(a.color_a, b.color_a);
        cb = blend(a.color_b, b.color_b);
        da = blend(a.dir_a, b.dir_a);
        db = blend(a.dir_b, b.dir_b);

        // vrsqrt Q, vf0w, |d|^2; vmulq.xyz (w kept).
        for (V4* d : {&da, &db}) {
            const u32 q = ps2::rsqrt(ps2::kOne, ps2::dot3(*d, *d));
            *d = {ps2::mul((*d)[0], q), ps2::mul((*d)[1], q), ps2::mul((*d)[2], q), (*d)[3]};
        }
    }

    const u32 wa = ps2::add(0, ca[3]);
    const u32 wb = ps2::add(0, cb[3]);
    ca[3] = 0;
    cb[3] = 0;

    MergedPointLight point;

    if (points && points->bank != nullptr) {
        point = merge_point_lights(
            ps2::bits(tie_instance_centre(tie, inst)), *points->bank, points->list
        );
    }

    V4 dp = point.direction;
    V4 cp = point.color;
    const u32 wp = ps2::add(0, cp[3]);
    cp[3] = 0;
    dp[3] = 0;

    // vf4..vf6 = the rows of -N (N = the unit columns), then L = vf4 * d.x +
    // vf5 * d.y + vf6 * d.z, i.e. L = -N^T d: the light in class space,
    // pointing toward the light.
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
    InstanceLightRegs regs;
    regs.colors = {ca, cb, cp};
    regs.back = {wa, wb, wp};

    // The transpose: vf24 = (La.x, Lb.x, Lp.x, 0), vf25 the y's, vf26 the z's.
    for (std::size_t i = 0; i < 3; ++i) {
        regs.rows[i] = {la[i], lb[i], lp[i], 0};
    }

    return regs;
}

std::array<u8, 4> light_tie_slot(
    const InstanceLightRegs& regs, const std::array<s16, 4>& normal, u16 ambient
) {
    return light_instance_normal(regs, normal, color_floats(pext5(ambient)), kInstanceColorClamp);
}

std::array<std::array<u8, 4>, kTieLightSlots> light_tie_instance(
    const TieClass& tie,
    const TieInstance& inst,
    const LightBank& bank,
    const std::optional<TiePointLights>& points
) {
    const InstanceLightRegs regs = tie_light_regs(tie, inst, bank, points);
    std::array<std::array<u8, 4>, kTieLightSlots> out{};

    for (std::size_t j = 0; j < kTieLightSlots; ++j) {
        const std::array<s16, 4> n = j < tie.normals.size() ? tie.normals[j] : std::array<s16, 4>{};
        out[j] = light_tie_slot(regs, n, inst.ambient_rgbas[j]);
    }

    return out;
}

}  // namespace openrac::assets::rac1
