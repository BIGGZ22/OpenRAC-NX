// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/hud.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// A level's HUD graphics: the hud_header lump and its five hud banks.
// Specification: ReRAC's docs/plan/hud_text.md section 1. Addresses are
// NTSC-U level01.
//
// The header (copied to the HUD heap by LoadHudBanks, 0x253e28) holds four
// tables, every offset from the header:
// - icons ({u16 id, u16 frame_count, u16 first_frame, u8 anim_mode, u8
//   ticks_per_frame}, ending with id 0xffff): Hud_GetIconIndex (0x24a4e0)
//   scans them; GetIconFrame (0x24fe10) turns (icon, k) into a frame;
// - frames ({s16 palette, s16 texture});
// - palettes ({u32 offset | 0x80000000, u16 runtime CBP, pad}): 256 RGBA32
//   entries in CSM1 order at `offset` in their bank;
// - textures ({u32 offset | 0x80000000, u16 runtime TBP, u8 log2 width, u8
//   log2 height}): 8-bit indices at `offset` in their bank.
// Bank b owns palettes palette_total[b - 1] .. palette_total[b] and the same
// for textures (LinkHudBank, 0x24a848, clears bit 31 and adds the bank's
// address). The banks are WAD-compressed on the disc; these readers take them
// decompressed.

#pragma once

#include <array>
#include <optional>
#include <utility>
#include <vector>

#include "assets/bytes.h"
#include "assets/image.h"
#include "assets/version.h"

namespace openrac::assets {

// The number of hud banks a level has.
inline constexpr std::size_t kHudBanks = 5;
inline constexpr std::size_t kHudHeaderSize = 0xb4;  // the icon table follows on every level

// hud_header +0x00..+0xb4. Every per-bank array has 8 entries of which the
// first 5 are used; +0x74 and +0x94 are run-time fields, 0 on the disc.
struct HudHeader {
    u16 icon_count = 0;  // including the terminator
    u16 frame_count = 0;
    u32 icon_offset = 0;
    u32 frame_offset = 0;
    u32 palette_offset = 0;
    u32 texture_offset = 0;
    std::array<u32, 8> palette_total{};  // +0x14: cumulative palettes per bank
    std::array<u32, 8> texture_total{};  // +0x34: cumulative textures per bank
    std::array<u32, 8> bank_size{};      // +0x54: decompressed size, 0 absent
    std::array<u32, 8> runtime_bank_base{};
    std::array<u32, 8> runtime_stash{};
};

// anim_mode drives the per-slot frame animation: 0 static, 1 loop, 2
// ping-pong, 3 ping-pong with a random pause.
struct HudIcon {
    u16 id = 0;
    u16 frame_count = 0;
    u16 first_frame = 0;
    u8 anim_mode = 0;
    u8 ticks_per_frame = 0;
};

// One frame: the palette and texture entries it draws with (-1 for none).
struct HudFrame {
    s16 palette = 0;
    s16 texture = 0;
};

// A palette entry: where its colours are in the bank (bit 31 is a flag the loader clears), and
// the GS address the game fills in at run time.
struct HudPalette {
    u32 offset_flags = 0;
    u16 cbp = 0;

    std::size_t offset() const { return offset_flags & 0x7fff'ffff; }
};

// A texture entry: where its pixels are in the bank, the GS address the game fills in at run
// time, and the log2 of each side.
struct HudTexture {
    u32 offset_flags = 0;
    u16 tbp = 0;
    u8 log2_width = 0;
    u8 log2_height = 0;

    std::size_t offset() const { return offset_flags & 0x7fff'ffff; }

    u32 width() const { return u32{1} << (log2_width & 0x1f); }

    u32 height() const { return u32{1} << (log2_height & 0x1f); }
};

// Reads the header's fixed part; throws when `game`'s layout is unknown or `header` is short.
HudHeader read_hud_header(Game game, ByteView header);

// A level's HUD set with copies of its decompressed banks.
class HudSet {
public:
    // Absent banks are empty views.
    static HudSet read(Game game, ByteView header, const std::array<ByteView, kHudBanks>& banks);

    const HudHeader& header() const { return m_header; }

    const std::vector<HudIcon>& icons() const { return m_icons; }

    const std::vector<HudFrame>& frames() const { return m_frames; }

    const std::vector<HudPalette>& palettes() const { return m_palettes; }

    const std::vector<HudTexture>& textures() const { return m_textures; }

    // The bank owning palette i / texture i.
    std::optional<std::size_t> palette_bank(std::size_t i) const;
    std::optional<std::size_t> texture_bank(std::size_t i) const;

    // Hud_GetIconIndex: a linear scan; a missing id gives the terminator's index.
    std::size_t icon_index(u16 id) const;

    // GetIconFrame(id, k) with every bank resident: first_frame + k when the
    // icon exists and k < frame_count, else frame 0. (The game also gives 0
    // while the frame's bank is not loaded.)
    std::size_t icon_frame(u16 id, s32 k) const;

    // The size of frame i in texels.
    std::optional<std::pair<u32, u32>> frame_size(std::size_t i) const;

    // Frame i's indices and palette, borrowed from this set.
    IndexedImage frame_image(std::size_t i) const;

    // Frame i as RGBA, with `alpha` choosing how the GS alpha maps to 0..255.
    RgbaImage decode_frame(std::size_t i, GsAlpha alpha) const;

private:
    // The header and its four tables, copied so the set outlives the lump it was read from.
    HudHeader m_header;
    std::vector<HudIcon> m_icons;
    std::vector<HudFrame> m_frames;
    std::vector<HudPalette> m_palettes;
    std::vector<HudTexture> m_textures;
    // The decompressed banks, copied; an absent bank is empty.
    std::array<std::vector<u8>, kHudBanks> m_banks;
};

}  // namespace openrac::assets
