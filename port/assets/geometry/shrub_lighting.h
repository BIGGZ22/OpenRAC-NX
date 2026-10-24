// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/shrub_light.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// RAC1 shrub instance lighting: the game's LightShrubs (NTSC-U level01
// 0x29e7e8; the boot copy is not located) and the VU0 routines it drives
// (program 436083, the same code as the tie entries on other registers), bit
// for bit (ReRAC docs/plan/shrub_lighting.md).
//
// Per instance the pass lights the class's 24 normals once into a 24-entry
// palette, which VU1 program 56467 indexes with each vertex's normal index.
// Like LightTies it rotates the light directions into class space; the
// differences, all read in the disassembly:
//
//   - the columns are normalised with c * rsqrt(|c|^2) on VU0, not the
//     loader's 1.0 / length;
//   - a blended light set scales only the xyz lanes, so the blended back-face
//     factor is the plain sum of both sets' factors;
//   - the ambient is one colour per instance, alpha 0x80;
//   - the point-light origin is the instance's world bounding-sphere centre.

#pragma once

#include <optional>

#include "assets/geometry/lighting.h"
#include "assets/geometry/shrub.h"

namespace openrac::assets::rac1 {

// One instance's lit colours (RGBA bytes), indexed by a vertex's normal index.
using ShrubPalette = std::array<std::array<u8, 4>, kShrubNormals>;

// The ambient word the loader stores in the matrix block's col0.w: b << 16 |
// g << 8 | r | 0x80000000 on the raw s32 channels (ORed, not masked: a channel
// above 255 spills into the next byte; the disc's are 0..255).
u32 shrub_packed_ambient(const ShrubInstance& instance);

// The lengths of the matrix's three axis columns, as float bits.
std::array<u32, 3> shrub_column_lengths(const ShrubInstance& instance);

// The instance's world bounding sphere as the loader stores it: M3x3 * class
// bsphere.xyz, radius = bsphere.w * the longest column, both times the class
// scale, plus the translation. So the class bounding sphere is in units of
// `scale` world units, as Wrench's builder assumes.
std::array<f32, 4> shrub_instance_centre(const ShrubClass& shrub, const ShrubInstance& instance);

// Point lights for one instance: the slot bank and the run-time record's nibble list (0xffff =
// none, the load-time value).
struct ShrubPointLights {
    const PointLightBank* bank = nullptr;
    u16 list = 0xffff;
};

// The VU0 registers LightShrubs sets up before lighting a class's normals: the shared light
// registers plus the instance's ambient.
struct ShrubLightRegs {
    InstanceLightRegs lights;
    ps2::V4 ambient{};  // vf31: 65536 + c / 128 per lane, alpha lane 0x80

    bool operator==(const ShrubLightRegs&) const = default;
};

// The EE half of LightShrubs for one instance: light set select and blend, the point-light merge
// and the rotation of the light directions into class space.
ShrubLightRegs shrub_light_regs(
    const ShrubClass& shrub,
    const ShrubInstance& instance,
    const LightBank& bank,
    const std::optional<ShrubPointLights>& points = std::nullopt
);

// One class normal lit with the instance's registers (RGB <= 243, alpha 0x80).
std::array<u8, 4> light_shrub_normal(const ShrubLightRegs& regs, const std::array<s16, 4>& normal);

// LightShrubs for one instance: the 24 palette colours by normal index (0x80
// = 1.0, RGB <= 243, alpha 0x80). Without point lights this is the level-load
// pass (the loader sets every list to 0xffff).
ShrubPalette light_shrub_instance(
    const ShrubClass& shrub,
    const ShrubInstance& instance,
    const LightBank& bank,
    const std::optional<ShrubPointLights>& points = std::nullopt
);

// The loader's column-1 w word: the per-channel integer mean of the palette,
// which ShrubProc colours billboards with.
std::array<u8, 3> shrub_average_colour(const ShrubPalette& palette);

// --- Where VU1 program 56467 reads a vertex's colour

// The size of VU1 data memory in quadwords; addresses wrap at this value.
constexpr u16 kShrubVu1Qwc = 0x400;
// The two instance buffers (ShrubProc alternates them per MSCALF batch).
constexpr std::array<u16, 2> kShrubVu1InstanceBuffers = {0xee, 0x17b};
// The size of one instance slot in quadwords: 4 matrix columns + 24 palette entries.
constexpr u16 kShrubVu1SlotQwc = 0x1c;
// The number of instance slots in one buffer, so how many instances one batch holds.
constexpr std::size_t kShrubVu1Batch = 5;

// The palette base of instance k of a batch in the buffer at `buffer`:
// buffer + 1 + 0x1c k (the slot) + 4 (the matrix).
u16 shrub_vu1_palette_base(u16 buffer, std::size_t k);

// The VU1 address a vertex reads its colour from. A quirk (ReRAC
// shrub_sky_rac1.md 1.3b): when the stop flag is on vertex 2 (a packet of 6
// written vertices), the loop leaves with the address already formed for
// vertex 3 and adds the palette base again, so vertex 3 reads n + 2 * base.
// Only its RGB is affected; alpha stays the instance's.
u16 shrub_vu1_colour_address(
    std::size_t written_vertices, std::size_t vertex, u8 normal, u16 palette_base
);

// What an address of VU1 data memory holds while the shrub program runs.
struct ShrubVu1Data {
    enum class Kind : u8 {
        Palette,  // palette entry `index` of instance slot `slot` of `buffer`
        Matrix,   // matrix column `index` of instance slot `slot`
        Other,    // constants, input buffers, a batch count, a GS output buffer
    };

    Kind kind = Kind::Other;
    u16 buffer = 0;
    std::size_t slot = 0;
    u16 index = 0;  // the entry or column; for Other, the address

    bool operator==(const ShrubVu1Data&) const = default;
};

// Says what `address` holds: which buffer, slot and entry when it is inside an instance slot.
ShrubVu1Data shrub_vu1_data(u16 address);

}  // namespace openrac::assets::rac1
