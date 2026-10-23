// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/level.rs
// (the record types the geometry readers take): ISC License, Copyright (c) 2026 ReRAC
// contributors.
//
// The level core index records the geometry readers need: texture table
// entries, class table entries with their texture slots, shrub billboard
// descriptors and gadget entries. They are read here only as records; finding
// the tables in a level is the level reader's job (assets/disc), which hands
// the geometry readers these records and the blocks they point at.
//
// RAC1 layouts (both the PAL and NTSC-U discs; the editor reads the same from
// PAL). The other games' are not known yet.

#pragma once

#include <array>
#include <vector>

#include "assets/bytes.h"

namespace openrac::assets::rac1 {

// A tfrag, moby, tie or shrub texture table entry (0x10 bytes).
struct CoreTextureEntry {
    s32 data_offset = 0;  // 0x0: pixel bytes in the core data's textures block
    s16 width = 0;        // 0x4: pixels
    s16 height = 0;       // 0x6: pixels
    s16 levels = 0;       // 0x8: 1..4; for tfrag textures the mip level count (TEX1 MXL + 1)
    s16 palette = 0;      // 0xa: CLUT at gs_ram[palette * 0x100], 1024 bytes
    s16 mip2_block = 0;   // 0xc: tfrag textures: gs_ram block (0x100 bytes) of mip level 2
    s16 mip3_block = 0;   // 0xe: tfrag textures: gs_ram block of mip level 3, -1 with 3 levels
};

static_assert(sizeof(CoreTextureEntry) == 0x10);

// A moby or tie class table entry (0x20 bytes).
struct CoreClassEntry {
    s32 offset = 0;   // 0x00: the class blob in the core data; 0 = no blob
    s32 o_class = 0;  // 0x04: the class number
    s32 unknown_8 = 0;
    s32 unknown_c = 0;
    // 0x10: the class's GS texture slots, each an index into the moby, tie or
    // shrub texture table; 0xff = unused.
    std::array<u8, 16> textures{};

    // The texture table index of a slot, or -1 when the slot is unused.
    int texture_table_index(int slot) const {
        if (slot < 0 || slot >= 16 || textures[static_cast<std::size_t>(slot)] == 0xff) {
            return -1;
        }

        return textures[static_cast<std::size_t>(slot)];
    }
};

static_assert(sizeof(CoreClassEntry) == 0x20);

// A shrub class's far-LOD billboard texture: blocks of 0x100 bytes in gs_ram.
struct CoreBillboardInfo {
    s16 width = 0;
    s16 height = 0;
    s16 levels = 0;  // mip level count (TEX1 MXL + 1)
    s16 palette_block = 0;
    s16 texture_block = 0;
    s16 mip1_block = 0;
    s16 mip2_block = 0;
    s16 mip3_block = 0;
};

static_assert(sizeof(CoreBillboardInfo) == 0x10);

// A shrub class table entry (0x30 bytes): a class entry and its billboard.
struct CoreShrubClassEntry {
    CoreClassEntry base;
    CoreBillboardInfo billboard;
};

static_assert(sizeof(CoreShrubClassEntry) == 0x30);

// A gadget table entry (0x10 bytes): one WAD-compressed moby class blob.
struct CoreGadgetEntry {
    s32 offset = 0;           // the compressed stream in the core data
    s32 o_class = 0;          // the class number
    s32 compressed_size = 0;  // the stream's size (its own header repeats it)
    s32 pad = 0;
};

static_assert(sizeof(CoreGadgetEntry) == 0x10);

// `count` records of T at `offset` in a core index; none when either is not
// positive (the core index's convention for an absent table).
template <typename T>
std::vector<T> read_core_table(ByteView index, s32 count, s32 offset, std::string_view what) {
    if (count <= 0 || offset <= 0) {
        return {};
    }

    if (count > 100'000) {
        fail("{}: implausible count {}", what, count);
    }

    return index
        .read_array<T>(static_cast<std::size_t>(offset), static_cast<std::size_t>(count), what);
}

}  // namespace openrac::assets::rac1
