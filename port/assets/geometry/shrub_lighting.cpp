// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/shrub_light.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The implementation of shrub_lighting.h: LightShrubs on PS2 float arithmetic, and VU1 program
// 56467's memory map.
//
// The file has two parts: the lighting (resolve the instance's light sets, merge its point lights,
// rotate the directions into class space, then light the class's 24 normals into a palette) and
// the small address helpers that say where in VU1 data memory a vertex's palette colour lives.
// All functions are pure and safe to call from any thread.

#include "assets/geometry/shrub_lighting.h"

#include <algorithm>

namespace openrac::assets::rac1 {

namespace {

using ps2::V4;

// Unlike the tie pass, the columns are normalised on VU0 as c * rsqrt(|c|^2). The w lane (the
// loader's per-instance words) is left alone.
// vmul.xyz; vadday.x; vmaddz.x (|c|^2), vrsqrt Q, vf0w, vmulq.xyz.
std::array<V4, 3> unit_columns(const ShrubInstance& inst) {
    std::array<V4, 3> out{};

    // Normalise each of the three axis columns.
    for (std::size_t i = 0; i < 3; ++i) {
        const V4 c = ps2::bits(inst.matrix[i]);
        const u32 q = ps2::rsqrt(ps2::kOne, ps2::dot3(c, c));
        out[i] = {ps2::mul(c[0], q), ps2::mul(c[1], q), ps2::mul(c[2], q), c[3]};
    }

    return out;
}

}  // namespace

// The channels are ORed together without masking, as the loader does, so a channel above 255
// would spill into the next byte; the disc's values never do.
u32 shrub_packed_ambient(const ShrubInstance& inst) {
    const auto r = static_cast<u32>(inst.colour[0]);
    const auto g = static_cast<u32>(inst.colour[1]);
    const auto b = static_cast<u32>(inst.colour[2]);
    return (b << 16 | g << 8 | r) | 0x8000'0000u;
}

// The length of a column is the square root of its x, y, z dot product (the w lane is ignored).
std::array<u32, 3> shrub_column_lengths(const ShrubInstance& inst) {
    std::array<u32, 3> out{};

    // One length per axis column.
    for (std::size_t i = 0; i < 3; ++i) {
        const V4 c = ps2::bits(inst.matrix[i]);
        out[i] = ps2::sqrt(ps2::dot3(c, c));
    }

    return out;
}

// Same as the tie instance's centre, but the shrub matrix has no w lane to transform, so only the
// xyz lanes are accumulated.
std::array<f32, 4> shrub_instance_centre(const ShrubClass& shrub, const ShrubInstance& inst) {
    std::array<V4, 4> m{};

    // Load the four matrix rows (three axis columns and the translation) as raw float bits.
    for (std::size_t i = 0; i < 4; ++i) {
        m[i] = ps2::bits(inst.matrix[i]);
    }

    const V4 b = ps2::bits(shrub.header.bsphere);
    V4 c{};

    // Transform the sphere centre by the three axis columns, lane by lane.
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

// Mirrors LightShrubs' set-up of the VU0 registers. The selector is the low half of the
// instance's light word: one set, or a blend of two with the weight in bits 8..15.
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

    // A zero high byte means one plain set; anything else is a blend of two.
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

        // Blending shortens a direction, so renormalise both.
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

    // Without a point-light bank the merged light stays zero and contributes nothing.
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
    ShrubLightRegs regs;
    regs.lights.colors = {ca, cb, cp};
    regs.lights.back = {wa, wb, wp};

    // The transpose: row i holds component i of each light's class-space direction.
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

// Lights one class normal with the instance's registers; the ambient is the instance's one colour.
std::array<u8, 4> light_shrub_normal(const ShrubLightRegs& regs, const std::array<s16, 4>& normal) {
    return light_instance_normal(regs.lights, normal, regs.ambient, kInstanceColorClamp);
}

// Slots past the class's normal count are lit with a zero normal, so they get only ambient.
ShrubPalette light_shrub_instance(
    const ShrubClass& shrub,
    const ShrubInstance& inst,
    const LightBank& bank,
    const std::optional<ShrubPointLights>& points
) {
    const ShrubLightRegs regs = shrub_light_regs(shrub, inst, bank, points);
    ShrubPalette out{};

    // Light each of the class's normals into its palette entry.
    for (std::size_t j = 0; j < kShrubNormals; ++j) {
        const std::array<s16, 4> n =
            j < shrub.normals.size() ? shrub.normals[j] : std::array<s16, 4>{};
        out[j] = light_shrub_normal(regs, n);
    }

    return out;
}

// The mean is an integer division of the per-channel sums, truncating as the loader does.
std::array<u8, 3> shrub_average_colour(const ShrubPalette& palette) {
    std::array<u32, 3> sum{};

    // Sum every palette colour per channel.
    for (const auto& c : palette) {
        // Add the red, green and blue channels (alpha is not averaged).
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

// The base is the buffer's start plus 5 (a header quadword and the four matrix columns) plus one
// slot's size for each earlier instance of the batch.
u16 shrub_vu1_palette_base(u16 buffer, std::size_t k) {
    // iaddiu vi13, vi13, 5 after the matrix loads, then + 0x1c per instance.
    return static_cast<u16>(buffer + 5 + kShrubVu1SlotQwc * k);
}

// The address is the palette base plus the normal index, wrapped to VU1 data memory; the stop-flag
// quirk described in the header adds the base a second time for vertex 3 of a six-vertex packet.
u16 shrub_vu1_colour_address(
    std::size_t written_vertices, std::size_t vertex, u8 normal, u16 palette_base
) {
    const u32 extra = written_vertices == 6 && vertex == 3 ? palette_base : 0u;
    return static_cast<u16>((u32{normal} + palette_base + extra) % kShrubVu1Qwc);
}

// Classifies an address by finding which instance buffer's batch of slots contains it; anything
// outside both buffers is reported as Other with the wrapped address.
ShrubVu1Data shrub_vu1_data(u16 address) {
    const auto a = static_cast<u16>(address % kShrubVu1Qwc);

    // Try each of the two instance buffers the program alternates between.
    for (const u16 buffer : kShrubVu1InstanceBuffers) {
        const auto rel = static_cast<u16>(a - buffer);

        // Quadword 0 of a buffer is not part of any slot, so slots start at offset 1.
        if (rel >= 1 && rel <= kShrubVu1SlotQwc * kShrubVu1Batch) {
            const std::size_t slot = (rel - 1u) / kShrubVu1SlotQwc;
            const auto qw = static_cast<u16>((rel - 1u) % kShrubVu1SlotQwc);

            // A slot is four matrix columns followed by the 24 palette entries.
            if (qw < 4) {
                return {ShrubVu1Data::Kind::Matrix, buffer, slot, qw};
            }

            return {ShrubVu1Data::Kind::Palette, buffer, slot, static_cast<u16>(qw - 4)};
        }
    }

    return {ShrubVu1Data::Kind::Other, 0, 0, a};
}

}  // namespace openrac::assets::rac1
