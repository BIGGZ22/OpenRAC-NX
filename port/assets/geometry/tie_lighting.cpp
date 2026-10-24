// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tie_light.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The implementation of tie_lighting.h: LightTies on PS2 float arithmetic.
//
// The pass has one expensive part, tie_light_regs, which resolves the instance's light sets,
// merges its point lights and rotates the light directions into class space. The per-slot part is
// then the shared light_instance_normal of lighting.h, run once for each of the 64 slots. All
// functions are pure and safe to call from any thread.

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

    // Scale each column by its reciprocal length and keep the reciprocal in w.
    for (std::size_t i = 0; i < 3; ++i) {
        const u32 inv = ps2::div(ps2::kOne, lengths[i]);
        const V4 c = ps2::bits(inst.matrix[i]);
        out[i] = {ps2::mul(c[0], inv), ps2::mul(c[1], inv), ps2::mul(c[2], inv), inv};
    }

    return out;
}

}  // namespace

// The length of a column is the square root of its x, y, z dot product (the w lane is ignored).
std::array<u32, 3> tie_column_lengths(const TieInstance& inst) {
    std::array<u32, 3> out{};

    // One length per axis column.
    for (std::size_t i = 0; i < 3; ++i) {
        const V4 c = ps2::bits(inst.matrix[i]);
        out[i] = ps2::sqrt(ps2::dot3(c, c));
    }

    return out;
}

// The matrix's translation row is added after the class scale, so the scale does not move the
// instance.
std::array<f32, 4> tie_instance_centre(const TieClass& tie, const TieInstance& inst) {
    std::array<V4, 4> m{};

    // Load the four matrix rows (three axis columns and the translation) as raw float bits.
    for (std::size_t i = 0; i < 4; ++i) {
        m[i] = ps2::bits(inst.matrix[i]);
    }

    const V4 b = ps2::bits(tie.header.bsphere);
    // vmulax / vmadday / vmaddaz with the three columns, vmaddw with vf0.
    V4 c{};

    // Transform the sphere centre lane by lane; lane 3 gets the translation's w through vf0.
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

// Mirrors the EE half of LightTies. The selector is the low half of the instance's directional
// light word, in the same encoding as a tfrag's (one set, or a blend of two).
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

    // A zero high byte means one plain set; anything else is a blend of two.
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

        // Blending shortens a direction, so renormalise both.
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

    // Without a point-light bank the merged light stays zero and contributes nothing.
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

        // Each class-space component is the dot of one negated unit column with the direction.
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

// The slot's ambient is RGBA5551 and is widened with pext5 before the shared lighting step.
std::array<u8, 4> light_tie_slot(
    const InstanceLightRegs& regs, const std::array<s16, 4>& normal, u16 ambient
) {
    return light_instance_normal(regs, normal, color_floats(pext5(ambient)), kInstanceColorClamp);
}

// Slots past the class's normal count are lit with a zero normal, so they get only ambient.
std::array<std::array<u8, 4>, kTieLightSlots> light_tie_instance(
    const TieClass& tie,
    const TieInstance& inst,
    const LightBank& bank,
    const std::optional<TiePointLights>& points
) {
    const InstanceLightRegs regs = tie_light_regs(tie, inst, bank, points);
    std::array<std::array<u8, 4>, kTieLightSlots> out{};

    // Light each of the 64 slots with its class normal and this instance's ambient.
    for (std::size_t j = 0; j < kTieLightSlots; ++j) {
        const std::array<s16, 4> n = j < tie.normals.size() ? tie.normals[j] : std::array<s16, 4>{};
        out[j] = light_tie_slot(regs, n, inst.ambient_rgbas[j]);
    }

    return out;
}

}  // namespace openrac::assets::rac1
