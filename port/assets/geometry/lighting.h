// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tfrag_light.rs
// (the light bank, point lights, the normal table and the ELF reader): ISC License,
// Copyright (c) 2026 ReRAC contributors.
//
// The inputs every RAC1 baked-lighting pass shares (ReRAC
// docs/plan/tfrag_lighting.md): the level's directional light sets from the
// gameplay file, the point-light slots gameplay code fills at run time, and
// the 256-entry (cos, sin) table the passes decode packed normals with.
//
// The table is data of the game's executable: its values are not IEEE-rounded
// cos/sin (296 of the 512 floats differ in the last bits), so a bit-exact
// pass needs the player's own copy. NormalTable::from_elf reads it from the
// boot ELF at run time; nothing of it is stored here. NormalTable::computed()
// is a stand-in for tests and for renderers that do not need bit-exactness.
// EE addresses are NTSC-U (SCUS_971.99).

#pragma once

#include <array>

#include "assets/bytes.h"
#include "common/ps2_float.h"

namespace openrac::assets::rac1 {

// Directional light sets in the EE bank (FastMemZero16(0x180340, 0x400)).
constexpr std::size_t kLightBankSets = 16;
// The level loader caps the gameplay file's light count at 12.
constexpr std::size_t kMaxLevelLights = 12;
// Point-light slots at EE 0x180740, 0x20 bytes each.
constexpr std::size_t kPointLightSlots = 8;

// Boot-ELF address of the normal table in the NTSC-U boot ELF (the boot copy
// of LightTfrags loads it with lui/addiu 0x165500; every level overlay
// carries an identical copy).
constexpr u32 kNtscNormalTableAddress = 0x0016'5500;

// One directional light set (0x40 bytes): two lights, each a colour and the
// direction it travels. colour.w is the back-face factor: the dot product d
// becomes max(d, d * w).
struct DirLightSet {
    std::array<f32, 4> color_a{};  // 1.0 adds 128 to a colour byte
    std::array<f32, 4> dir_a{};
    std::array<f32, 4> color_b{};
    std::array<f32, 4> dir_b{};
};

static_assert(sizeof(DirLightSet) == 0x40);

// The EE directional light bank; sets past the level's count are zero.
struct LightBank {
    std::array<DirLightSet, kLightBankSets> sets{};
    std::size_t count = 0;  // sets the gameplay file provided, after the cap of 12
};

// The directional lights of a decompressed gameplay file: the u32 at 0x04 is
// the section offset; the section is an s32 count, 12 bytes of padding and
// `count` DirLightSet records.
LightBank parse_light_bank(ByteView gameplay);

// One point light slot (EE 0x180740 + 0x20 * slot), built at run time by
// gameplay code with colour = rgb / 128.
struct PointLight {
    std::array<f32, 4> color{};     // w: back-face factor
    std::array<f32, 4> position{};  // world, w = radius
};

static_assert(sizeof(PointLight) == 0x20);

using PointLightBank = std::array<PointLight, kPointLightSlots>;

// 256 (cos, sin) pairs for angle i * 2 pi / 256 as raw float bits.
struct NormalTable {
    std::array<std::array<u32, 2>, 256> entries{};

    // The table at `address` in a little-endian ELF32 (the boot ELF).
    static NormalTable from_elf(ByteView elf, u32 address = kNtscNormalTableAddress);

    // IEEE cos/sin, exact at the quarter turns. Not the game's table.
    static NormalTable computed();
};

// `length` bytes at virtual address `address` from the PT_LOAD segments of a
// little-endian ELF32.
ByteView elf_read(ByteView elf, u32 address, std::size_t length);

// The VU0 registers the tie and shrub passes (LightTies, LightShrubs) set up
// for VU0 program 436083 before lighting a class's normals for one instance.
struct InstanceLightRegs {
    // vf27, vf28, vf29: the colours of light A, light B and the merged point
    // light, w cleared.
    std::array<ps2::V4, 3> colors{};
    // vf30.xyz: each light's back-face factor (max(d, d * w)).
    std::array<u32, 3> back{};
    // vf24, vf25, vf26: the x, y and z components of the three class-space
    // "to light" vectors (lane k = light k).
    std::array<ps2::V4, 3> rows{};

    bool operator==(const InstanceLightRegs&) const = default;
};

// VU0 436083 for one class normal (the tie entries 0x58 / 0x84, the shrub
// entries 0 / 0x2c): itof15 of the normal, three dot products, the back-face
// max, ambient * 1.0 + A * dA + B * dB + P * dP, minii.xyz with `clamp`; then
// the EE's ppach / ppacb keep each lane's low byte. `ambient` holds the
// floats 65536 + c / 128.
std::array<u8, 4> light_instance_normal(
    const InstanceLightRegs& regs,
    const std::array<s16, 4>& normal,
    const ps2::V4& ambient,
    u32 clamp
);

// The colour clamp of VU0 436083 (its I register): 65536 + 243 / 128. Tie and
// shrub colours saturate at 243, not 255.
constexpr u32 kInstanceColorClamp = 0x4780'00f3;

// The point lights the tie and shrub passes merge into their third light:
// the sum of unit vectors (centre - light) and of colours * (1 - dist / r)
// over the lights in `list` (low nibble first, 0xf ends) whose radius reaches
// `centre`, the direction renormalised only when at least two contributed.
struct MergedPointLight {
    ps2::V4 direction{};
    ps2::V4 color{};
};

MergedPointLight merge_point_lights(const ps2::V4& centre, const PointLightBank& bank, u16 list);

// The colour bytes as floats 65536 + c / 128 (pextlb / pextlh to words,
// padduw with 0x47800000).
ps2::V4 color_floats(const std::array<u8, 4>& color);

// PEXT5 of a 5:5:5:1 colour: r = bits 0..4 << 3, g = 5..9 << 3, b = 10..14 << 3,
// a = bit 15 << 7.
std::array<u8, 4> pext5(u16 color);

}  // namespace openrac::assets::rac1
