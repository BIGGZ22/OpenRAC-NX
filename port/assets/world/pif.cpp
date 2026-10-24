// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/pif.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// Implements pif.h: reads a PIF header and hands out its palette and pixels without copying. The
// header is checked first (magic, format, size), so a wrong offset or a damaged lump fails here
// with the offset in the message instead of producing a garbage picture.

#include "assets/world/pif.h"

#include <bit>
#include <cstring>

#include "assets/world/known_games.h"

namespace openrac::assets {

namespace {

// The largest texture side accepted (assumed: the GS texture limit).
constexpr u32 kPifMaxSide = 1024;

// 256 palette entries of 4 bytes each.
constexpr std::size_t kPifPaletteBytes = 0x400;

}  // namespace

// The header fields are at fixed offsets (see pif.h); +4 is the file size, which is not needed.
PifImage read_pif(Game game, ByteView bytes, std::size_t at) {
    require_world_layout(game, "PIF image");
    const ByteView magic = bytes.sub(at, 4, "PIF magic");

    // The magic tells a PIF from any other data at this offset.
    if (std::memcmp(magic.data(), "2FIP", 4) != 0) {
        fail("no PIF at {:#x}", at);
    }

    PifImage pif;
    pif.width = bytes.u32_at(at + 8);
    pif.height = bytes.u32_at(at + 0xc);
    pif.format = bytes.u32_at(at + 0x10);

    // The only format the game's texture setup handles is 8-bit indices with a 32-bit palette.
    if (pif.format != kPifFormatPsmt8) {
        fail("PIF at {:#x}: format {:#x} is not PSMT8", at, pif.format);
    }

    // The GS takes the log2 of each side, so a side is a power of two within the size limit.
    if (!std::has_single_bit(pif.width) || !std::has_single_bit(pif.height)
        || pif.width > kPifMaxSide || pif.height > kPifMaxSide) {
        fail("PIF at {:#x}: {}x{}", at, pif.width, pif.height);
    }

    pif.palette = bytes.sub(at + kPifPaletteOffset, kPifPaletteBytes, "PIF palette");
    pif.pixels =
        bytes.sub(at + kPifPixelsOffset, std::size_t{pif.width} * pif.height, "PIF pixels");
    return pif;
}

}  // namespace openrac::assets
