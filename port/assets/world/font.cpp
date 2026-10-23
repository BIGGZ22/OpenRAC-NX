// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

// Implements font.h: finds each face's glyph table in a level's program and measures text with it.
// The tables have no fixed address, so the program's code is scanned for the call sites that load
// them (a `li a0, face` in a call's delay slot, then a lui/addiu pair into t2 whose addiu sits in
// the delay slot of the printer call). Every site of a face must give the same address.

#include "assets/world/font.h"

#include <cstring>

namespace openrac::assets::rac1 {

namespace {

// MIPS encodings the call sites are made of.

// A jal, the call to the printer or to its texture setup.
bool is_call(u32 word) {
    return word >> 26 == 3;  // jal
}

// The face number a `li a0, n` word loads, as an addiu or an ori from zero.
std::optional<u32> load_a0(u32 word) {
    const u32 op = word & 0xffff'0000;

    // addiu or ori a0, zero, n: the two ways the compiler writes a small constant load.
    if (op == 0x2404'0000 || op == 0x3404'0000) {  // addiu / ori a0, zero, n
        return word & 0xffff;
    }

    return std::nullopt;
}

// A lui into t2, the high half of the table address.
bool is_lui_t2(u32 word) {
    return (word & 0xffff'0000) == 0x3c0a'0000;
}

// An addiu t2, t2, lo, the low half of the table address.
bool is_addiu_t2(u32 word) {
    return (word & 0xffff'0000) == 0x254a'0000;
}

// The face and table address a call site at words[at] (the li a0 in a delay slot) loads, if it
// is one. A few words after it, a lui and an addiu build the table's address.
std::optional<std::pair<u32, u32>> call_site(std::span<const u32> words, std::size_t at) {
    const auto face = load_a0(words[at]);

    // Only faces 1 to 3 exist, and the li must follow a call (it is in that call's delay slot).
    if (!face || *face < 1 || *face > 3 || at == 0 || !is_call(words[at - 1])) {
        return std::nullopt;
    }

    // The lui comes within the next three words; the first one found is the address's high half.
    for (std::size_t hi = at + 1; hi < std::min(at + 4, words.size()); ++hi) {
        // Words between the call and the lui are other code; keep looking.
        if (!is_lui_t2(words[hi])) {
            continue;
        }

        // The addiu is within the next dozen words, in the delay slot of the printer call.
        for (std::size_t lo = hi + 1; lo < std::min(hi + 12, words.size()); ++lo) {
            // Combine the halves; the addiu's immediate is sign extended.
            if (is_addiu_t2(words[lo]) && is_call(words[lo - 1])) {
                const u32 address =
                    ((words[hi] & 0xffff) << 16)
                    + static_cast<u32>(static_cast<s32>(static_cast<s16>(words[lo] & 0xffff)));
                return std::pair{*face, address};
            }
        }
        return std::nullopt;
    }
    return std::nullopt;
}

}  // namespace

// Scans the executable sections of the level program for the call sites of each face.
std::array<u32, 3> locate_glyph_tables(std::span<const disc::OverlaySection> sections) {
    std::array<std::optional<u32>, 3> found{};

    // Only code sections can hold the call sites.
    for (const disc::OverlaySection& s : sections) {
        // Kind 1 is the code kind (assumed from the overlay layout).
        if (s.kind != 1) {
            continue;
        }
        std::vector<u32> words(s.data.size() / 4);
        std::memcpy(words.data(), s.data.data(), words.size() * 4);

        // Try every word as the start of a call site.
        for (std::size_t i = 0; i < words.size(); ++i) {
            const auto site = call_site(words, i);

            // Most words are not call sites.
            if (!site) {
                continue;
            }
            auto& slot = found[site->first - 1];

            // Two sites of one face that disagree mean the scan matched something else.
            if (slot && *slot != site->second) {
                fail(
                    "font {}: call sites disagree on its glyph table ({:#x}, {:#x})",
                    site->first,
                    *slot,
                    site->second
                );
            }
            slot = site->second;
        }
    }
    std::array<u32, 3> out{};

    // Every face must have been found at least once.
    for (std::size_t f = 0; f < 3; ++f) {
        // A face with no call site has no table, so nothing could print with it.
        if (!found[f]) {
            fail("font {}: no call site loads its glyph table", f + 1);
        }
        out[f] = *found[f];
    }
    return out;
}

// Copies each face's table out of the program's memory image.
LevelFonts read_level_fonts(ByteView overlay) {
    const auto sections = disc::parse_overlay_sections(overlay);
    LevelFonts fonts;
    fonts.addresses = locate_glyph_tables(sections);

    // One table per face.
    for (std::size_t f = 0; f < 3; ++f) {
        const auto bytes =
            disc::read_overlay(sections, fonts.addresses[f], kGlyphCount * sizeof(GlyphCell));

        // The address must fall inside the program's loaded sections.
        if (!bytes) {
            fail(
                "font {}: glyph table {:#x} is not in the level program", f + 1, fonts.addresses[f]
            );
        }
        std::memcpy(fonts.tables[f].data(), bytes->data(), kGlyphCount * sizeof(GlyphCell));

        // Every face has an 'A': a cheap proof the call sites led to tables.
        if (!fonts.tables[f]['A'].present()) {
            fail("font {}: the table at {:#x} has no 'A'", f + 1, fonts.addresses[f]);
        }
    }
    return fonts;
}

// Bytes of 0xe8 and above have no glyph and are skipped, as the game does.
s32 text_width(std::string_view text, const GlyphTable& table, std::optional<std::size_t> limit) {
    s32 width = 0;

    // Add up the advances; the NUL or the byte limit ends the text.
    for (std::size_t i = 0; i < text.size(); ++i) {
        const auto c = static_cast<u8>(text[i]);

        // The text ends at a NUL or at the caller's limit.
        if (c == 0 || (limit && i >= *limit)) {
            break;
        }

        // A byte past the table draws nothing and moves nothing.
        if (c < kGlyphCount) {
            width += table[c].advance;
        }
    }
    return width;
}

}  // namespace openrac::assets::rac1
