// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/transition.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The transition lump's parts.

#include "assets/disc/transition.h"

#include <algorithm>

#include "assets/disc/wad.h"

namespace openrac::assets::disc {

TransitionLump TransitionLump::parse(ByteView lump) {
    TransitionLump t;
    t.m_bytes = is_wad(lump) ? wad_decompress(lump) : lump.to_vector();
    t.m_base = ByteView(t.m_bytes).u32_at(4);

    // A base of zero or past the end would send every later offset outside the lump.
    if (t.m_base == 0 || t.m_base > t.m_bytes.size()) {
        fail("transition lump: data base {:#x}", t.m_base);
    }

    return t;
}

std::size_t TransitionLump::word(std::size_t i) const {
    return ByteView(m_bytes).u32_at(4 * i);
}

ByteView TransitionLump::variant(std::size_t v) const {
    // The lump holds a fixed number of flight variants; a larger index has no data.
    if (v >= kVariants) {
        fail("transition lump: no flight variant {}", v);
    }

    const std::size_t at = m_base + word(0x14 + v);
    const std::size_t end = m_base + word(v + 1 < kVariants ? 0x15 + v : 0x19);
    return ByteView(m_bytes).sub(at, end > at ? end - at : 0, "flight variant");
}

ByteView TransitionLump::sky_block() const {
    const std::size_t at = m_base + word(0x12);
    const std::size_t end = m_base + word(0x13);
    return ByteView(m_bytes).sub(at, end > at ? end - at : 0, "flight sky");
}

ByteView TransitionLump::picture(std::size_t k) const {
    const ByteView b(m_bytes);
    const std::size_t table = m_base + word(0x13);
    const u32 count = b.u32_at(table);

    // The picture table holds count entries; a larger index would read past it.
    if (k >= count) {
        fail("transition lump: picture {} of {}", k, count);
    }

    return b.tail(table + b.u32_at(table + 4 + 4 * k), "flight picture");
}

ByteView TransitionLump::planet_picture(s32 level) const {
    // Only the first kLevels pictures are planet pictures; the rest are captions.
    if (level < 0 || static_cast<std::size_t>(level) >= kLevels) {
        fail("transition lump: no picture for level {}", level);
    }

    return picture(static_cast<std::size_t>(level));
}

ByteView TransitionLump::caption(s32 language, s32 level) const {
    // Captions exist per level only, so an index outside the level count has none.
    if (level < 0 || static_cast<std::size_t>(level) >= kLevels) {
        fail("transition lump: no caption for level {}", level);
    }

    const auto l = static_cast<std::size_t>(std::max(language - 1, 0));
    return picture(kLevels + kLevels * l + static_cast<std::size_t>(level));
}

std::vector<TransitionLump::FxEntry> TransitionLump::fx_entries() const {
    const std::size_t count = word(0xa);
    const std::size_t table = word(0xb);

    // The FX texture table is small; a large count means the lump is damaged.
    if (count > 64) {
        fail("transition lump: {} FX textures", count);
    }

    return ByteView(m_bytes).read_array<FxEntry>(table, count, "FX texture table");
}

ByteView TransitionLump::fx_bank() const {
    const std::size_t at = std::min(m_base + word(0xe), m_bytes.size());
    return ByteView(m_bytes).tail(at);
}

ByteView TransitionLump::sound_bank() const {
    return ByteView(m_bytes).tail(m_base + word(0x19), "flight sound bank");
}

std::vector<std::pair<s32, std::size_t>> TransitionLump::classes() const {
    const ByteView b(m_bytes);
    const std::size_t count = std::min<std::size_t>(word(4), 16);
    const std::size_t table = word(5);
    std::vector<std::pair<s32, std::size_t>> out;

    // Read one class record per entry, up to the 16 the lump can hold.
    for (std::size_t i = 0; i < count; ++i) {
        out.emplace_back(b.s32_at(table + 32 * i + 4), b.u32_at(table + 32 * i));
    }

    return out;
}

ByteView TransitionLump::gs_image() const {
    const std::size_t at = word(0);
    return ByteView(m_bytes).sub(at, m_base > at ? m_base - at : 0, "flight GS image");
}

}  // namespace openrac::assets::disc
