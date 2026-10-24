// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/toc.rs and
// crates/rc-formats/src/disc.rs: ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The per-version layouts and the RAC1 global header's fields.

#include "assets/disc/toc.h"

#include <algorithm>
#include <cstring>

#include "assets/disc/wad.h"

namespace openrac::assets::disc {

namespace {

constexpr FieldKind kRange = FieldKind::SectorRange;
constexpr FieldKind kBytes = FieldKind::SectorByteRange;
constexpr FieldKind kSector = FieldKind::Sector;

// The RAC1 global header in table order, without the level table. The names
// are the extracted file names (global/<name>...); they follow Wrench's
// RacWadInfo, as ReRAC's and editor/disc.py's do, with the epilogue split by
// language as ReRAC splits it.
constexpr GlobalField kRac1GlobalFields[] = {
    {"debug_font", 0x0008, 1, kRange},
    {"save_game", 0x0010, 1, kRange},
    {"ratchet_seqs", 0x0018, 28, kRange},
    {"hud_seqs", 0x00f8, 20, kRange},
    {"vendor", 0x0198, 1, kRange},
    {"vendor_audio", 0x01a0, 37, kRange},
    {"help_controls", 0x02c8, 12, kRange},
    {"help_moves", 0x0328, 15, kRange},
    {"help_weapons", 0x03a0, 15, kRange},
    {"help_gadgets", 0x0418, 14, kRange},
    {"help_ss", 0x0488, 7, kRange},
    {"options_ss", 0x04c0, 7, kRange},
    {"frontbin", 0x04f8, 1, kRange},
    {"mission_ss", 0x0500, 81, kRange},
    {"planets", 0x0788, 19, kRange},
    {"unknown_0820", 0x0820, 38, kRange},
    {"goodies_images", 0x0950, 10, kRange},
    {"character_sketches", 0x09a0, 19, kRange},
    {"character_renders", 0x0a38, 19, kRange},
    {"skill_images", 0x0ad0, 31, kRange},
    {"epilogue_english", 0x0bc8, 12, kRange},
    {"epilogue_french", 0x0c28, 12, kRange},
    {"epilogue_italian", 0x0c88, 12, kRange},
    {"epilogue_german", 0x0ce8, 12, kRange},
    {"epilogue_spanish", 0x0d48, 12, kRange},
    {"sketchbook", 0x0da8, 30, kRange},
    {"commercials", 0x0e98, 4, kRange},
    {"item_images", 0x0eb8, 9, kRange},
    {"qwark_boss_audio", 0x0f00, 240, kSector},
    {"irx", 0x12c0, 1, kRange},
    {"spaceships", 0x12c8, 4, kRange},
    {"unknown_12e8", 0x12e8, 20, kRange},
    {"space_plates", 0x1388, 6, kRange},
    {"transition", 0x13b8, 1, kRange},
    {"space_audio", 0x13c0, 36, kRange},
    {"sound_bank", 0x14e0, 1, kRange},
    {"unknown_14e8", 0x14e8, 1, kRange},
    {"music", 0x14f0, 1, kRange},
    {"hud_header", 0x14f8, 1, kRange},
    {"hud_banks", 0x1500, 5, kRange},
    {"all_text", 0x1528, 1, kRange},
    {"unknown_1530", 0x1530, 28, kRange},
    {"post_credits_helpdesk_girl_seq", 0x1610, 1, kRange},
    {"post_credits_audio", 0x1618, 18, kRange},
    {"credits_images_ntsc", 0x16a8, 20, kRange},
    {"credits_images_pal", 0x1748, 20, kRange},
    {"unknown_17e8", 0x17e8, 2, kRange},
    {"mpegs", 0x17f8, 88, kBytes},
    {"help_audio", 0x1ab8, 900, kSector},
};

constexpr DiscLayout kRac1 = {
    Game::Rac1,
    true,
    1500,
    kRac1TocSize,
    0x28c8,
    19,
    kRac1LevelHeaderSize,
    kRac1GlobalFields,
    "NTSC-U: ReRAC's docs/formats/disc_layout.md; PAL: OpenRAC's editor/disc.py",
};

// No table of contents is read for these yet. games/rac4/ntsc/docs/RESEARCH.md
// places RAC4's at sector 1001 but does not describe its layout;
// games/rac2/ntsc/docs/LEVEL-ARCHIVE-FORMAT.md describes a prototype's
// RC2.HDR, not the retail disc.
constexpr DiscLayout kUnknown[] = {
    {Game::Rac2, false, 0, 0, 0, 0, 0, {}, "RAC2's table of contents is not known yet"},
    {Game::Rac3, false, 0, 0, 0, 0, 0, {}, "RAC3's table of contents is not known yet"},
    {Game::Rac4, false, 0, 0, 0, 0, 0, {}, "RAC4's table of contents is not known yet"},
};

}  // namespace

const DiscLayout& rac1_layout() {
    return kRac1;
}

const DiscLayout& disc_layout(const GameVersion& version) {
    // Only RAC1 has a known layout so far; the other versions return a layout marked unknown.
    switch (version.game) {
        case Game::Rac1:
            return kRac1;
        case Game::Rac2:
            return kUnknown[0];
        case Game::Rac3:
            return kUnknown[1];
        case Game::Rac4:
            return kUnknown[2];
    }

    return kUnknown[2];
}

std::span<const s32> SceneRecord::chunk_sectors(bool pal_region) const {
    const auto& all = pal_region ? pal : ntsc;
    const auto zero = std::find(all.begin(), all.end(), 0);
    return {all.data(), static_cast<std::size_t>(zero - all.begin())};
}

std::optional<std::pair<u32, u32>> SceneRecord::region_sectors(bool pal_region) const {
    const std::span<const s32> s = chunk_sectors(pal_region);

    // A level with no chunks in this region (PAL or NTSC) has no scene run.
    if (s.empty()) {
        return std::nullopt;
    }

    return std::pair{static_cast<u32>(s.front()), static_cast<u32>(s.back() - s.front()) + 1};
}

LevelHeader parse_level_header(ByteView bytes) {
    const auto h = bytes.read<LevelHeader>(0, "level header");

    // The header's own size field is its signature; any other value is not a RAC1 level header.
    if (h.header_size != static_cast<s32>(kRac1LevelHeaderSize)) {
        fail("level header lacks the 0x2434 signature");
    }

    return h;
}

std::vector<std::optional<u32>> level_header_sectors(ByteView toc, const DiscLayout& layout) {
    // An unknown layout has no table offsets to read from.
    if (!layout.known) {
        fail("{}", layout.source);
    }

    // The table of contents starts with a version number, which must be 1.
    if (toc.s32_at(0) != 1) {
        fail("table of contents: version {} is not 1", toc.s32_at(0));
    }

    const s32 size = toc.s32_at(4);

    // The header size must cover the 8-byte preamble and fit the table.
    if (size <= 8 || static_cast<std::size_t>(size) > toc.size()) {
        fail("table of contents: implausible header size {:#x}", size);
    }

    std::vector<std::optional<u32>> out;

    // Read one sector number per level slot in the level table.
    for (u32 i = 0; i < layout.level_table_count; ++i) {
        const s32 s = toc.s32_at(layout.level_table_offset + i * 8);

        // A negative sector is a damaged table.
        if (s < 0) {
            fail("table of contents: negative level sector in slot {}", i);
        }

        out.push_back(s == 0 ? std::nullopt : std::optional<u32>(static_cast<u32>(s)));
    }

    return out;
}

SectorRange global_sector_range(ByteView toc, u32 field) {
    return {toc.s32_at(field), toc.s32_at(field + 4)};
}

ProbedSize probe_lump_size(ByteView head) {
    // A VAG file starts with "VAGp", and its length is stored big-endian at 0x0c.
    if (head.size() >= 0x30 && std::memcmp(head.data(), "VAGp", 4) == 0) {
        const u32 be = head.u32_at(0x0c);
        const u32 size = (be >> 24) | ((be >> 8) & 0xff00) | ((be << 8) & 0xff0000) | (be << 24);
        return {0x30 + u64{size}, true};
    }

    // A WAD lump carries its compressed size in its header.
    if (is_wad(head)) {
        return {wad_compressed_size(head), true};
    }

    return {2048, false};
}

}  // namespace openrac::assets::disc
