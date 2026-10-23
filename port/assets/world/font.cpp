// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "assets/world/font.h"

#include <cstring>

namespace openrac::assets::rac1 {

namespace {

// MIPS encodings the call sites are made of.
bool is_call(u32 word) {
    return word >> 26 == 3;  // jal
}

std::optional<u32> load_a0(u32 word) {
    const u32 op = word & 0xffff'0000;

    if (op == 0x2404'0000 || op == 0x3404'0000) {  // addiu / ori a0, zero, n
        return word & 0xffff;
    }

    return std::nullopt;
}

bool is_lui_t2(u32 word) {
    return (word & 0xffff'0000) == 0x3c0a'0000;
}

bool is_addiu_t2(u32 word) {
    return (word & 0xffff'0000) == 0x254a'0000;
}

// The table a call site at words[at] (the li a0 in a delay slot) loads, if
// it is one.
std::optional<std::pair<u32, u32>> call_site(std::span<const u32> words, std::size_t at) {
    const auto face = load_a0(words[at]);

    if (!face || *face < 1 || *face > 3 || at == 0 || !is_call(words[at - 1])) {
        return std::nullopt;
    }

    for (std::size_t hi = at + 1; hi < std::min(at + 4, words.size()); ++hi) {
        if (!is_lui_t2(words[hi])) {
            continue;
        }

        for (std::size_t lo = hi + 1; lo < std::min(hi + 12, words.size()); ++lo) {
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

std::array<u32, 3> locate_glyph_tables(std::span<const disc::OverlaySection> sections) {
    std::array<std::optional<u32>, 3> found{};

    for (const disc::OverlaySection& s : sections) {
        if (s.kind != 1) {
            continue;
        }
        std::vector<u32> words(s.data.size() / 4);
        std::memcpy(words.data(), s.data.data(), words.size() * 4);

        for (std::size_t i = 0; i < words.size(); ++i) {
            const auto site = call_site(words, i);

            if (!site) {
                continue;
            }
            auto& slot = found[site->first - 1];

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

    for (std::size_t f = 0; f < 3; ++f) {
        if (!found[f]) {
            fail("font {}: no call site loads its glyph table", f + 1);
        }
        out[f] = *found[f];
    }
    return out;
}

LevelFonts read_level_fonts(ByteView overlay) {
    const auto sections = disc::parse_overlay_sections(overlay);
    LevelFonts fonts;
    fonts.addresses = locate_glyph_tables(sections);

    for (std::size_t f = 0; f < 3; ++f) {
        const auto bytes =
            disc::read_overlay(sections, fonts.addresses[f], kGlyphCount * sizeof(GlyphCell));

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

s32 text_width(std::string_view text, const GlyphTable& table, std::optional<std::size_t> limit) {
    s32 width = 0;

    for (std::size_t i = 0; i < text.size(); ++i) {
        const auto c = static_cast<u8>(text[i]);

        if (c == 0 || (limit && i >= *limit)) {
            break;
        }

        if (c < kGlyphCount) {
            width += table[c].advance;
        }
    }
    return width;
}

}  // namespace openrac::assets::rac1
