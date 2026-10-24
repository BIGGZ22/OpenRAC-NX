// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/gadget.rs
// (spec: docs/formats/moby_rac1.md section 0.4): ISC License, Copyright (c) 2026 ReRAC
// contributors.
//
// RAC1 gadget classes: Ratchet's hand-held weapons and gadgets, the wrench
// among them. The core index's gadget table (+0x80 count, +0x84 offset) gives
// for each a WAD stream inside the decompressed core data that unpacks to an
// ordinary moby class blob. The class also has a moby class table entry with
// no blob, whose 16 texture slots map into the moby texture table as any
// class's do.
//
// The game (NTSC-U level 1) copies the table into four arrays by gadget
// number when the level loads (0x258128), and unpacks a gadget into one of
// two alternating 0x18000-byte buffers when it is taken in hand
// (select_world_object_resource_tables 0x259788, from LoadHandGadget
// 0x297d70): only the gadget in Ratchet's hand, and the one it replaces, is
// resident.

#pragma once

#include <optional>
#include <span>
#include <utility>
#include <vector>

#include "assets/bytes.h"
#include "assets/geometry/core_records.h"
#include "assets/geometry/moby.h"
#include "assets/geometry/texture.h"

namespace openrac::assets::rac1 {

// Each of the game's two gadget buffers; every class on the disc fits.
inline constexpr std::size_t kGadgetBufferSize = 0x18000;

// Ratchet's moby class (slot 0).
inline constexpr s32 kRatchetClass = 0;

// The wrench, a gadget class on every level.
inline constexpr s32 kWrenchClass = 71;

// Ratchet's joint list the hand item hangs from (every hand item but the
// glove types).
inline constexpr std::size_t kHandJointList = 0;

// One gadget: its table entry, the unpacked class data and the class parsed from it.
struct GadgetClass {
    CoreGadgetEntry entry;
    std::vector<u8> blob;  // the unpacked class (its pointers are relative to it)
    LevelMobyClass moby;
};

// Every gadget class of a level, in gadget-table order. Throws when a stream
// is not a WAD of the table's size or a class has no moby class table entry.
std::vector<GadgetClass> parse_gadget_classes(
    std::span<const CoreGadgetEntry> gadgets,
    std::span<const CoreClassEntry> moby_classes,
    ByteView core_data
);

// One used texture slot of a moby class.
struct ClassTexture {
    std::size_t slot = 0;            // the class's GS texture slot (MobyTriangle::texture)
    std::size_t index = 0;           // into the moby texture table
    std::optional<RgbaImage> image;  // none for an entry without pixels
};

// The moby textures a class uses, in slot order. `textures_block` is the core
// data from its textures base on; `gs_ram` the level's gs_ram lump.
std::vector<ClassTexture> class_textures(
    std::span<const CoreTextureEntry> moby_textures,
    ByteView textures_block,
    ByteView gs_ram,
    const CoreClassEntry& entry
);

// The two byte lists of joint list `list` of a class (header `joints`: s32
// count, s32 pointer[count]; each list s16 n1, s16 n2, u8 a[n1], u8 b[n2],
// 0xff). The first is a root-to-joint chain whose last entry is the joint an
// attachment hangs from (NTSC-U boot 0x20cca8 and 0x210850).
std::pair<std::vector<u8>, std::vector<u8>> joint_list(
    ByteView blob, const MobyClassHeader& header, std::size_t list
);

// The joint attachment list `list` resolves to.
u8 attachment_joint(ByteView blob, const MobyClassHeader& header, std::size_t list);

}  // namespace openrac::assets::rac1
