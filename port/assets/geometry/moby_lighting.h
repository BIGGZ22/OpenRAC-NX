// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/moby_light.rs
// (spec: docs/plan/moby_skinning_lighting.md sections 2, 4 and 5,
// docs/plan/moby_render_notes.md): ISC License, Copyright (c) 2026 ReRAC contributors.
//
// RAC1 moby instance rotation and vertex lighting, bit for bit in the PS2's
// float arithmetic (ps2_float.h). Three of the game's routines (NTSC-U):
//
//   rotation_rows  0x20def8 runs a VU0 routine (program 28259) on the
//                  instance's Euler angles (moby +0x40) and stores the rows at
//                  moby +0xc0, +0xd0, +0xe0: from the identity, for each
//                  non-zero angle, Rx, then Ry, then Rz, so row i = R e_i with
//                  R = Rz Ry Rx. Sine and cosine come from an odd 9th-order
//                  polynomial after folding the angle into [-pi/2, pi/2].
//   moby_lights    MobyProc (0x211808, 0x211d88..0x212140) builds a moby's
//                  light block: the selected light sets' directions taken into
//                  model space (L = -R^T d), their colours, back-light factors
//                  and the ambient from moby +0x3c.
//   light_vertex   VU0 program 104691's single-matrix loop per vertex, then
//                  the EE pack of 0x1ee650 (multiplier bytes, halfword
//                  saturation).
//
// The point-light merge (MobyProc 0x212150) is per frame and the renderer's;
// this load-time pass leaves the third light at zero, which is what the game
// holds when no point light is in range.

#pragma once

#include <array>
#include <span>
#include <utility>
#include <vector>

#include "assets/geometry/lighting.h"
#include "assets/geometry/moby.h"
#include "common/ps2_float.h"

namespace openrac::assets::rac1 {

using Rows = std::array<ps2::V4, 3>;

// The identity joint matrix (rows 0..2): the bind pose.
inline constexpr Rows kIdentityRows = {
    ps2::V4{ps2::kOne, 0, 0, 0},
    ps2::V4{0, ps2::kOne, 0, 0},
    ps2::V4{0, 0, ps2::kOne, 0},
};

// (sin a, cos a) as the VU0 routine computes them.
std::pair<u32, u32> vu0_sin_cos(u32 angle);

// The rotation rows for Euler angles (radians), as the VU0 routine builds them.
Rows rotation_rows(const std::array<f32, 3>& angles);

// The rows of an instance: rotation_rows, and mode bit 0x8000 negates row 1
// (mirrored mobys).
Rows instance_rows(const std::array<f32, 3>& angles, u16 mode);

// A moby's light block as 0x1ee650 loads it into VU0.
struct MobyLights {
    Rows rows{};        // row j = (L0[j], L1[j], L2[j], 0): the model-space "to light" vectors
    Rows colors{};      // the lights' colours, w = 0 (1.0 adds 128 to a colour byte)
    ps2::V4 neg_k{};    // (-|K0|, -|K1|, -|K2|, 1): K the colours' w, the back-light factor
    ps2::V4 ambient{};  // 0x47800000 | byte per lane: 65536 + {r, g, b, a} / 128

    bool operator==(const MobyLights&) const = default;
};

// The light block of a moby with rotation rows `rows`, light word moby +0x38
// ([set 0, set 1, fade, -]), ambient moby +0x3c and alpha (0x80 when fully
// visible). Sets past the bank read as zero, as the game's unused ones are.
MobyLights moby_lights(
    const Rows& rows,
    const LightBank& bank,
    u32 light_word,
    const std::array<u8, 3>& ambient,
    u8 alpha
);

// The lit colour of one vertex with joint matrix rows `m` (the blended palette
// matrix; kIdentityRows for the bind pose) and the vertex's RGBA multiplier
// (0x80 = 1.0).
std::array<u8, 4> light_vertex(
    const MobyLights& lights,
    const NormalTable& table,
    const Rows& m,
    u8 azimuth,
    u8 elevation,
    const std::array<u8, 4>& multiplier
);

// The EE pack: low halfword of each lane times the multiplier, saturated to
// s16, shifted right 7, low byte.
std::array<u8, 4> pack_colour(const ps2::V4& colour, const std::array<u8, 4>& multiplier);

// One LOD list (MobyClass::high_lod, for example) lit in the bind pose: one
// RGBA per vertex of each packet. Duplicate vertices are not lit again: as in
// the game they take the colour last written to their vertex-cache slot,
// carried across the list's packets.
std::vector<std::vector<std::array<u8, 4>>> light_lod(
    std::span<const MobyPacket> packets, const MobyLights& lights, const NormalTable& table
);

}  // namespace openrac::assets::rac1
