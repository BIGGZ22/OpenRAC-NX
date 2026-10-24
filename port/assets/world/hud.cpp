// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/hud.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// Implements hud.h: parses the HUD header and its four tables, keeps copies of the banks, and
// answers which bank holds an entry and what a frame looks like. Every table is bounds checked
// against the header lump and every frame lookup against the tables, so a damaged level fails
// with a message naming the table instead of reading past a buffer.

#include "assets/world/hud.h"

#include "assets/world/known_games.h"

namespace openrac::assets {

namespace {

// 256 palette entries of 4 bytes each.
constexpr std::size_t kHudPaletteBytes = 0x400;

// The bank that owns entry `i`, given the cumulative totals: bank b owns totals[b - 1] up to
// totals[b]. Nothing when `i` is past the last bank's total.
std::optional<std::size_t> owning_bank(const std::array<u32, 8>& totals, std::size_t i) {
    // The totals only grow, so the first bank whose total is past `i` owns it.
    for (std::size_t b = 0; b < kHudBanks; ++b) {
        // Entry i is below this bank's running total, so it is one of this bank's.
        if (i < totals[b]) {
            return b;
        }
    }

    return std::nullopt;
}

}  // namespace

// The scalar fields are at +0x00..+0x10, then five arrays of eight words each from +0x14 (see
// hud.h).
HudHeader read_hud_header(Game game, ByteView b) {
    require_world_layout(game, "HUD header");
    b.check(0, kHudHeaderSize, "HUD header");
    HudHeader h;
    h.icon_count = b.u16_at(0);
    h.frame_count = b.u16_at(2);
    h.icon_offset = b.u32_at(4);
    h.frame_offset = b.u32_at(8);
    h.palette_offset = b.u32_at(0xc);
    h.texture_offset = b.u32_at(0x10);

    // The arrays are eight entries long on disc, though only the first five banks are used.
    for (std::size_t k = 0; k < 8; ++k) {
        h.palette_total[k] = b.u32_at(0x14 + 4 * k);
        h.texture_total[k] = b.u32_at(0x34 + 4 * k);
        h.bank_size[k] = b.u32_at(0x54 + 4 * k);
        h.runtime_bank_base[k] = b.u32_at(0x74 + 4 * k);
        h.runtime_stash[k] = b.u32_at(0x94 + 4 * k);
    }

    return h;
}

// Reads the four tables, then copies the banks. The last bank's totals give the number of
// palettes and textures, since the totals are cumulative.
HudSet HudSet::read(Game game, ByteView b, const std::array<ByteView, kHudBanks>& banks) {
    HudSet hud;
    hud.m_header = read_hud_header(game, b);
    const HudHeader& h = hud.m_header;
    const std::size_t palettes = h.palette_total[kHudBanks - 1];
    const std::size_t textures = h.texture_total[kHudBanks - 1];

    // Sanity limits: a header outside them is not a HUD header, and the counts size allocations.
    if (h.icon_count == 0 || h.icon_count > 0x1000 || h.frame_count > 0x4000 || palettes > 0x1000
        || textures > 0x1000) {
        fail("HUD header: implausible counts");
    }

    b.check(h.icon_offset, std::size_t{h.icon_count} * 8, "HUD icon table");

    // Each icon entry is 8 bytes: id, frame count, first frame, animation mode, ticks per frame.
    for (std::size_t i = 0; i < h.icon_count; ++i) {
        const std::size_t at = h.icon_offset + 8 * i;
        hud.m_icons.push_back(
            {b.u16_at(at), b.u16_at(at + 2), b.u16_at(at + 4), b.u8_at(at + 6), b.u8_at(at + 7)}
        );
    }

    // The game's scan stops at id 0xffff, so a table without it would run on.
    if (hud.m_icons.back().id != 0xffff) {
        fail("HUD icon table has no 0xffff terminator");
    }

    b.check(h.frame_offset, std::size_t{h.frame_count} * 4, "HUD frame table");

    // Each frame entry is 4 bytes: palette index, texture index.
    for (std::size_t i = 0; i < h.frame_count; ++i) {
        hud.m_frames.push_back(
            {b.s16_at(h.frame_offset + 4 * i), b.s16_at(h.frame_offset + 4 * i + 2)}
        );
    }

    b.check(h.palette_offset, palettes * 8, "HUD palette table");

    // Each palette entry is 8 bytes: flagged offset, run-time CBP, padding.
    for (std::size_t i = 0; i < palettes; ++i) {
        const std::size_t at = h.palette_offset + 8 * i;
        hud.m_palettes.push_back({b.u32_at(at), b.u16_at(at + 4)});
    }

    b.check(h.texture_offset, textures * 8, "HUD texture table");

    // Each texture entry is 8 bytes: flagged offset, run-time TBP, log2 width, log2 height.
    for (std::size_t i = 0; i < textures; ++i) {
        const std::size_t at = h.texture_offset + 8 * i;
        hud.m_textures.push_back(
            {b.u32_at(at), b.u16_at(at + 4), b.u8_at(at + 6), b.u8_at(at + 7)}
        );
    }

    // Keep a copy of each bank, so the set does not depend on the caller's buffers.
    for (std::size_t i = 0; i < kHudBanks; ++i) {
        // A bank shorter than the header says would make the entries point outside it.
        if (banks[i].size() < h.bank_size[i]) {
            fail("HUD bank {} is shorter than its header size", i);
        }

        hud.m_banks[i] = banks[i].to_vector();
    }

    return hud;
}

// See owning_bank: palettes are counted by palette_total.
std::optional<std::size_t> HudSet::palette_bank(std::size_t i) const {
    return owning_bank(m_header.palette_total, i);
}

// See owning_bank: textures are counted by texture_total.
std::optional<std::size_t> HudSet::texture_bank(std::size_t i) const {
    return owning_bank(m_header.texture_total, i);
}

// The game's scan: the first icon with this id, or the terminator when there is none.
std::size_t HudSet::icon_index(u16 id) const {
    // The terminator ends the scan, so a missing id lands on it.
    for (std::size_t i = 0; i < m_icons.size(); ++i) {
        // Stop at the wanted id, or at the terminator when the id is not in the table.
        if (m_icons[i].id == 0xffff || m_icons[i].id == id) {
            return i;
        }
    }

    return m_icons.size() - 1;
}

// The frame index of animation step k of icon `id`; frame 0 when the icon or step is absent.
std::size_t HudSet::icon_frame(u16 id, s32 k) const {
    const HudIcon& icon = m_icons[icon_index(id)];

    // The terminator (a missing icon) or a step past the icon's frames has no frame.
    if (icon.id == 0xffff || k >= icon.frame_count) {
        return 0;
    }

    const s32 frame = icon.first_frame + k;
    return frame < 0 ? 0 : static_cast<std::size_t>(frame);
}

// Nothing for a frame that does not exist or has no texture.
std::optional<std::pair<u32, u32>> HudSet::frame_size(std::size_t i) const {
    // A missing frame, or one whose texture index is -1, has no size.
    if (i >= m_frames.size() || m_frames[i].texture < 0) {
        return std::nullopt;
    }

    const auto t = static_cast<std::size_t>(m_frames[i].texture);

    // A texture index past the table is damaged data; report it as no size.
    if (t >= m_textures.size()) {
        return std::nullopt;
    }

    return std::pair{m_textures[t].width(), m_textures[t].height()};
}

// Borrows the pixels and palette from the set's copies of the banks. Throws on any entry that
// points outside the tables or the banks.
IndexedImage HudSet::frame_image(std::size_t i) const {
    // A frame index past the table is a caller or data error.
    if (i >= m_frames.size()) {
        fail("HUD frame {} out of range", i);
    }

    const HudFrame& f = m_frames[i];

    // A frame with a -1 entry is not drawable; there are no pixels to hand out.
    if (f.palette < 0 || f.texture < 0) {
        fail("HUD frame {} has a negative palette or texture", i);
    }

    const auto pi = static_cast<std::size_t>(f.palette);
    const auto ti = static_cast<std::size_t>(f.texture);

    // Both indices must be inside their tables.
    if (pi >= m_palettes.size() || ti >= m_textures.size()) {
        fail("HUD frame {}: entry out of range", i);
    }

    const auto pb = palette_bank(pi);
    const auto tb = texture_bank(ti);

    // An entry past the last bank's total belongs to no bank, so it has no data.
    if (!pb || !tb) {
        fail("HUD frame {}: an entry owned by no bank", i);
    }

    const HudTexture& t = m_textures[ti];
    const ByteView pixels =
        ByteView(m_banks[*tb])
            .sub(t.offset(), std::size_t{t.width()} * t.height(), "HUD texture pixels");
    const ByteView palette =
        ByteView(m_banks[*pb]).sub(m_palettes[pi].offset(), kHudPaletteBytes, "HUD palette");
    return {t.width(), t.height(), pixels, palette};
}

// Convenience over frame_image().
RgbaImage HudSet::decode_frame(std::size_t i, GsAlpha alpha) const {
    return frame_image(i).decode(alpha);
}

}  // namespace openrac::assets
