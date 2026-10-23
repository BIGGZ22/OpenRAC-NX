// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Based on ReRAC's research (https://github.com/re-rac/rerac, docs/plan/hud_text.md,
// crates/rc-formats/src/font.rs): ISC License, Copyright (c) 2026 ReRAC contributors.
//
// RAC1's bitmap fonts. A font is one of the level's FX textures (1 regular,
// 2 small, 3 large; 256 x 128, particle_textures.h) and a table of 232 glyph
// cells, one per byte value, in the level program's data. The tables sit at
// a different address in each level's program, so they are found the way the
// game reaches them: every text-printing wrapper loads the FX texture with
// `li a0, n` in a call's delay slot, then the table's address into t2 with a
// lui and an addiu, the addiu in the delay slot of the call to the printer
// (NTSC-U level 1: the wrappers near 0x21cf70, the printer 0x21ccf0).
//
// Text bytes 0x08..0x0f are colour codes (0x08 the caller's colour, 0x09
// blue, 0x0a green, 0x0b purple, 0x0c orange for names in help text,
// 0x0d..0x0f black); the colours are in the level program, read with it.

#pragma once

#include <array>
#include <optional>
#include <span>
#include <string_view>

#include "assets/bytes.h"
#include "assets/disc/overlay.h"

namespace openrac::assets::rac1 {

// The three faces the game prints with, in the order of their FX textures.
enum class FontFace : u8 {
    Regular,  // HUD numbers, most menus
    Small,    // help boxes, subtitles
    Large,    // banners
};

// The FX texture a face draws from.
constexpr u32 font_fx_texture(FontFace face) {
    return static_cast<u32>(face) + 1;
}

// A glyph: its cell at texel (u, v) of the FX texture (16 x 16; 24 x 16 for
// the pad icons 0x10..0x1f), drawn `rise` lower than the line, then the pen
// moves by `advance`. A zero advance means the byte draws nothing.
struct GlyphCell {
    u8 u = 0;
    u8 v = 0;
    s8 rise = 0;
    s8 advance = 0;

    bool present() const { return advance != 0; }

    bool operator==(const GlyphCell&) const = default;
};

static_assert(sizeof(GlyphCell) == 4);

inline constexpr std::size_t kGlyphCount = 232;  // bytes 0x00..0xe7; accents from 0xc0

// One face's glyphs, indexed by the text byte.
using GlyphTable = std::array<GlyphCell, kGlyphCount>;

// The three glyph tables of one level, read from its program.
struct LevelFonts {
    std::array<GlyphTable, 3> tables{};
    std::array<u32, 3> addresses{};  // where each table is in the level program

    const GlyphTable& operator[](FontFace face) const {
        return tables[static_cast<std::size_t>(face)];
    }
};

// The table address of each face, from the printing wrappers' call sites.
// Throws when a face has none, or two disagree.
std::array<u32, 3> locate_glyph_tables(std::span<const disc::OverlaySection> sections);

// The three tables of a level program (its overlay lump).
LevelFonts read_level_fonts(ByteView overlay);

// The width the game measures for `text` (0x21cc40): the advances of its
// bytes up to the NUL or `limit` bytes. Bytes past the table count nothing.
s32 text_width(
    std::string_view text, const GlyphTable& table, std::optional<std::size_t> limit = {}
);

}  // namespace openrac::assets::rac1
