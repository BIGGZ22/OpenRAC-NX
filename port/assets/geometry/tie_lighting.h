// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tie_light.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// RAC1 tie instance lighting: the game's LightTies (NTSC-U boot 0x237370,
// level01 0x2ab218) and the VU0 routines it drives (program 436083), bit for
// bit (ReRAC docs/plan/tie_lighting.md).
//
// Ties are instanced, so the game lights the class's 64 light slots once per
// instance rather than the vertices: slot j has a class normal
// (TieClass::normals[j], s16 / 32768) and an instance ambient colour
// (TieInstance::ambient_rgbas[j], RGBA5551). The result is a 64-entry RGBA
// table per instance that VU1 indexes with each vertex's light slot.
//
// Instead of rotating the normals into world space, the pass rotates the (up
// to three) light directions into class space with the normalised transpose
// of the instance matrix. The colour arithmetic is LightTfrags' with a third
// light (the merged point lights) and a clamp at 243.

#pragma once

#include <optional>

#include "assets/geometry/lighting.h"
#include "assets/geometry/tie.h"

namespace openrac::assets::rac1 {

// The number of light slots in a tie class, and so of colours in an instance's lit table.
constexpr std::size_t kTieLightSlots = 64;

// The instance's world bounding sphere as the level loader stores it in the
// run-time record: M3x3 * bsphere.xyz, radius = bsphere.w * the longest axis
// column, both times the class scale, plus the translation.
std::array<f32, 4> tie_instance_centre(const TieClass& tie, const TieInstance& instance);

// FastVecLength (boot 0x1f9af0) of the matrix's three axis columns.
std::array<u32, 3> tie_column_lengths(const TieInstance& instance);

// Point lights for one instance: the slot bank and the run-time record's
// nibble list (0xffff = none, the load-time value).
struct TiePointLights {
    const PointLightBank* bank = nullptr;
    u16 list = 0xffff;
};

// The EE half of LightTies for one instance: light set select and blend
// (scaling all four lanes), the point-light merge, and the rotation of the
// light directions into class space through unit columns 1.0 / length (EE
// div.s).
InstanceLightRegs tie_light_regs(
    const TieClass& tie,
    const TieInstance& instance,
    const LightBank& bank,
    const std::optional<TiePointLights>& points = std::nullopt
);

// One slot: the normal and the RGBA5551 ambient.
std::array<u8, 4> light_tie_slot(
    const InstanceLightRegs& regs, const std::array<s16, 4>& normal, u16 ambient
);

// LightTies for one instance: the 64 lit colours by light slot (0x80 = 1.0,
// RGB <= 243, alpha = the ambient's bit 15 as 0x80 or 0).
std::array<std::array<u8, 4>, kTieLightSlots> light_tie_instance(
    const TieClass& tie,
    const TieInstance& instance,
    const LightBank& bank,
    const std::optional<TiePointLights>& points = std::nullopt
);

}  // namespace openrac::assets::rac1
