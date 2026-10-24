// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/toc.rs and
// crates/rc-formats/src/disc.rs (the global fields): ISC License, Copyright (c) 2026 ReRAC
// contributors.
//
// The disc's table of contents: where each game version keeps it, the global
// header's fields, the level table and the level headers it points at.
//
// RAC1 (NTSC-U and PAL): the game reads 6 sectors at sector 1500 and keeps
// 0x2960 bytes of them (NTSC-U: to EE 0x137b80; PAL: func_0012F3F8 into
// D_00137C80). The global header's fields (the global lumps) come first, the
// 19-slot level table at +0x28c8 last (PAL: D_0013A548, read by
// func_0012F4A8). Each slot names the sector of a 0x2434-byte level header.
// NTSC-U from ReRAC (docs/formats/disc_layout.md section 2); PAL from
// OpenRAC's own reader of the PAL disc, editor/disc.py, which gives the same
// sector, sizes, field offsets and level header layout.
//
// RAC2, RAC3 and RAC4: not known yet. Their versions are identified (serial
// and boot executable), but no table of contents is read: disc_layout()
// returns a layout whose `known` is false.

#pragma once

#include <array>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "assets/bytes.h"
#include "assets/version.h"

namespace openrac::assets::disc {

// {first sector, sector count}.
struct SectorRange {
    s32 offset;
    s32 size;

    bool empty() const { return offset == 0 && size == 0; }
};

// {first sector, byte count}.
struct SectorByteRange {
    s32 offset;
    s32 size;
};

// How a global-header field addresses its lumps.
enum class FieldKind : u8 {
    SectorRange,      // {sector, sectors}
    SectorByteRange,  // {sector, bytes}
    Sector,           // a bare sector; the size is probed (probe_lump_size)
};

// One field of the global header: `count` entries of `kind` at byte `offset`.
// A field with one entry is extracted as global/<name>.bin, else as
// global/<name>/NNN.bin.
struct GlobalField {
    std::string_view name;
    u32 offset;
    u32 count;
    FieldKind kind;
};

// Where a game version keeps its table of contents and how it is laid out.
struct DiscLayout {
    Game game;
    bool known;
    u32 toc_sector;
    u32 toc_size;            // bytes the game keeps
    u32 level_table_offset;  // in the table of contents
    u32 level_table_count;
    u32 level_header_size;
    std::span<const GlobalField> global_fields;
    std::string_view source;  // where the facts come from, or what is missing
};

// The layout of `version`'s disc. Never null; check `known`.
const DiscLayout& disc_layout(const GameVersion& version);

// RAC1's layout (both regions).
const DiscLayout& rac1_layout();

// RAC1: languages of a scene's speech (index = the game's language number):
// 0 English, 1 unused (0 on the disc), 2 French, 3 German, 4 Spanish, 5 Italian.
inline constexpr std::array<std::string_view, 6> kSceneLanguages =
    {"en", "l1", "fr", "de", "es", "it"};

// RAC1: chunk-sector slots per region of a scene record (the last used one is
// a 1-sector sentinel) and scene records per level header.
inline constexpr std::size_t kSceneChunkSlots = 71;
inline constexpr std::size_t kSceneRecords = 15;

// RAC1: one in-engine scene (cutscene) record of a level header, 0x250 bytes:
// the speech VAG sector per language, then the sectors of the NTSC and the
// PAL chunk WADs. A chunk's size is the sector difference to the next entry.
// Wrench reads the same bytes as 30 records {sounds[6], wads[68]}, an NTSC
// and a PAL half per scene.
struct SceneRecord {
    std::array<s32, 6> speech;
    std::array<s32, kSceneChunkSlots> ntsc;
    std::array<s32, kSceneChunkSlots> pal;

    // The region's chunk sectors, up to the first 0.
    std::span<const s32> chunk_sectors(bool pal_region) const;

    // {first sector, sector count} of the region's contiguous run: every
    // chunk plus the sentinel sector. Nothing when the region is empty.
    std::optional<std::pair<u32, u32>> region_sectors(bool pal_region) const;
};

static_assert(sizeof(SceneRecord) == 0x250);

// RAC1: the level header at the front of a level's sector run (0x2434 bytes).
struct LevelHeader {
    s32 id;
    s32 header_size;  // 0x2434: the header's signature
    SectorRange data;
    SectorRange gameplay_ntsc;
    SectorRange gameplay_pal;
    SectorRange occlusion;
    std::array<SectorByteRange, 36> bindata;
    std::array<s32, 15> music;
    std::array<SceneRecord, kSceneRecords> scenes;
};

static_assert(sizeof(LevelHeader) == 0x2434);

inline constexpr std::size_t kRac1LevelHeaderSize = 0x2434;
inline constexpr std::size_t kRac1TocSize = 0x2960;

// Throws unless the header carries the 0x2434 signature.
LevelHeader parse_level_header(ByteView bytes);

// The level table: the sector of each level header (nothing for an empty slot).
std::vector<std::optional<u32>> level_header_sectors(ByteView toc, const DiscLayout& layout);

// A SectorRange field of the global header at byte `field`.
SectorRange global_sector_range(ByteView toc, u32 field);

// RAC1: the global header's `save_game` field (the memory-card template).
inline constexpr u32 kSaveGameField = 0x10;

// The size of a lump addressed by a bare sector, from its first bytes: a VAG
// header gives 0x30 + its big-endian data size, a WAD header its compressed
// size, anything else one sector (`exact` false).
struct ProbedSize {
    u64 bytes;
    bool exact;
};

ProbedSize probe_lump_size(ByteView head);

// A level's audio or scene lump; `name` is its path under levels/NN/ without
// ".bin" (music/003, bindata/017, speech/05_en, scene/05_ntsc).
struct StreamLump {
    std::string name;
    u32 sector;
    u64 bytes;

    bool operator==(const StreamLump&) const = default;
};

// The bindata, music, speech and scene lumps of a level, in that order.
// `probe(sector)` returns probe_lump_size of that sector's first bytes. Per
// scene record k: speech/KK_<lang> (one VAG per language), then scene/KK_ntsc
// and scene/KK_pal (the region's whole run, chunks and sentinel).
template <typename Probe>
std::vector<StreamLump> level_stream_lumps(const LevelHeader& h, Probe&& probe) {
    std::vector<StreamLump> out;

    // Stream the level's bindata slots first, one lump per slot.
    for (std::size_t i = 0; i < h.bindata.size(); ++i) {
        const SectorByteRange& r = h.bindata[i];

        // A slot with no offset and no size is unused.
        if (r.offset == 0 && r.size == 0) {
            continue;
        }

        out.push_back(
            {std::format("bindata/{:03}", i), static_cast<u32>(r.offset), static_cast<u32>(r.size)}
        );
    }

    // Music lumps are named by sector only; their length is probed from the data.
    for (std::size_t i = 0; i < h.music.size(); ++i) {
        // A zero sector marks a missing music track.
        if (h.music[i] != 0) {
            const u32 s = static_cast<u32>(h.music[i]);
            out.push_back({std::format("music/{:03}", i), s, probe(s)});
        }
    }

    // Every scene has speech per language and a run of chunks per region.
    for (std::size_t k = 0; k < h.scenes.size(); ++k) {
        const SceneRecord& scene = h.scenes[k];

        // Each language has its own speech lump.
        for (std::size_t lang = 0; lang < kSceneLanguages.size(); ++lang) {
            // A zero sector means the scene has no speech in that language.
            if (scene.speech[lang] != 0) {
                const u32 s = static_cast<u32>(scene.speech[lang]);
                out.push_back(
                    {std::format("speech/{:02}_{}", k, kSceneLanguages[lang]), s, probe(s)}
                );
            }
        }

        // The NTSC and PAL discs hold different scene chunk runs.
        for (const bool pal : {false, true}) {
            // A run exists only if the scene has chunks in that region.
            if (const auto run = scene.region_sectors(pal)) {
                out.push_back(
                    {std::format("scene/{:02}_{}", k, pal ? "pal" : "ntsc"),
                     run->first,
                     u64{run->second} * 2048}
                );
            }
        }
    }

    return out;
}

}  // namespace openrac::assets::disc
