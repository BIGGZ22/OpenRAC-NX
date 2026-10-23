// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/scene.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The scene table and chunk parser.

#include "assets/disc/scene.h"

#include <algorithm>
#include <cstring>

#include "assets/disc/wad.h"

namespace openrac::assets::disc {

namespace {

constexpr std::size_t kSector = 2048;

}  // namespace

u32 ticks_per_chunk(SceneRegion region) {
    return region == SceneRegion::Pal ? 80 : 96;
}

std::string_view scene_region_name(SceneRegion region) {
    return region == SceneRegion::Pal ? "pal" : "ntsc";
}

std::optional<RegionTable> RegionTable::from_sectors(std::span<const s32> sectors) {
    // A scene with no chunks in this region has no table.
    if (sectors.empty()) {
        return std::nullopt;
    }

    // Check that the chunk sectors only ever go forward.
    for (std::size_t i = 0; i < sectors.size(); ++i) {
        // A zero sector, or one that does not follow the one before, is a damaged table.
        if (sectors[i] <= 0 || (i > 0 && sectors[i] <= sectors[i - 1])) {
            fail("scene chunk sectors are not strictly increasing");
        }
    }

    RegionTable t;

    // Each chunk runs from its sector to the next; the last sector only ends the final chunk.
    for (std::size_t i = 0; i + 1 < sectors.size(); ++i) {
        t.chunks.push_back(
            {static_cast<u32>(sectors[i]), static_cast<u32>(sectors[i + 1] - sectors[i])}
        );
    }

    t.sentinel = static_cast<u32>(sectors.back());
    return t;
}

u32 RegionTable::first_sector() const {
    return chunks.empty() ? sentinel : chunks.front().sector;
}

std::size_t RegionTable::file_bytes() const {
    return std::size_t{sentinel + 1 - first_sector()} * kSector;
}

std::optional<std::pair<std::size_t, std::size_t>> RegionTable::chunk_in_file(std::size_t i) const {
    // A chunk index past the table has no place in the file.
    if (i >= chunks.size()) {
        return std::nullopt;
    }

    const ChunkRange& c = chunks[i];
    return std::pair{
        std::size_t{c.sector - first_sector()} * kSector, std::size_t{c.sectors} * kSector
    };
}

SceneEntry SceneEntry::from_record(const SceneRecord& r) {
    SceneEntry e;

    // Record only the languages that have a speech lump; a zero sector means none.
    for (std::size_t i = 0; i < e.speech.size(); ++i) {
        // Zero marks a language with no speech.
        if (r.speech[i] > 0) {
            e.speech[i] = static_cast<u32>(r.speech[i]);
        }
    }

    e.ntsc = RegionTable::from_sectors(r.chunk_sectors(false));
    e.pal = RegionTable::from_sectors(r.chunk_sectors(true));
    return e;
}

const std::optional<RegionTable>& SceneEntry::region(SceneRegion r) const {
    return r == SceneRegion::Pal ? pal : ntsc;
}

bool SceneEntry::empty() const {
    return std::none_of(speech.begin(), speech.end(), [](const auto& s) { return s.has_value(); })
           && !ntsc && !pal;
}

std::vector<SceneEntry> scene_table(const LevelHeader& h) {
    std::vector<SceneEntry> out;

    // Convert every scene record in the level header.
    for (const SceneRecord& r : h.scenes) {
        out.push_back(SceneEntry::from_record(r));
    }

    return out;
}

std::vector<ByteView> lump_chunks(ByteView lump) {
    std::vector<ByteView> out;

    // The lump holds a table of up to 70 offset and size pairs.
    for (std::size_t i = 0; i < 70; ++i) {
        const s32 off = lump.s32_at(8 * i);
        const s32 size = lump.s32_at(8 * i + 4);

        // A zero size ends the table.
        if (size == 0) {
            break;
        }

        // Negative values would wrap when cast to unsigned; the table is damaged.
        if (off < 0 || size < 0) {
            fail("scene lump entry {}: offset {} size {}", i, off, size);
        }

        out.push_back(lump.sub(
            0x800 + static_cast<std::size_t>(off),
            static_cast<std::size_t>(size),
            "scene lump chunk"
        ));
    }

    return out;
}

ByteView SceneChunk::sequence(const SceneActor& actor) const {
    return ByteView(bytes).tail(actor.sequence_offset, "scene actor sequence");
}

std::size_t camera_records(s32 end_tick, std::size_t index, SceneRegion region) {
    const s32 tpc = static_cast<s32>(ticks_per_chunk(region));
    const s32 left = end_tick - static_cast<s32>(index) * tpc;

    // Every chunk must start before the scene's last tick.
    if (left <= 0) {
        fail("scene chunk {} starts at or after the end tick {}", index, end_tick);
    }

    return static_cast<std::size_t>(std::min(left, tpc)) + 1;
}

SceneChunk parse_scene_chunk(std::vector<u8> decompressed, std::size_t index, SceneRegion region) {
    SceneChunk c;
    c.bytes = std::move(decompressed);
    const ByteView b(c.bytes);
    c.header = b.read<ChunkHeader>(0, "scene chunk header");
    const std::size_t n_camera = camera_records(c.header.end_tick, index, region);

    // A negative offset would wrap when cast to unsigned.
    if (c.header.camera_offset < 0) {
        fail("scene chunk: negative camera offset");
    }

    c.camera = b.read_array<CamRecord>(
        static_cast<std::size_t>(c.header.camera_offset), n_camera, "scene camera table"
    );
    const std::vector<u32> offsets =
        b.read_array<u32>(0x14, c.header.actor_count, "scene actor offsets");

    // Read each actor named by the chunk's offset list.
    for (const u32 at : offsets) {
        SceneActor a;
        a.offset = at;
        a.klass = b.s32_at(at);
        a.unknown_04 = b.s32_at(at + 4);
        a.unknown_08 = b.s32_at(at + 8);
        a.track_offset = b.s32_at(at + 12);
        a.sequence_offset = at + 0x10;
        // The sequence header (0x1c bytes): frame count at +0x10, trigger
        // count at +0x12, the frame offsets after it.
        a.frame_count = b.u8_at(std::size_t{a.sequence_offset} + 0x10);
        a.trigger_count = b.u8_at(std::size_t{a.sequence_offset} + 0x12);

        // A negative offset would wrap when cast to unsigned.
        if (a.track_offset < 0) {
            fail("scene actor: negative position track offset");
        }

        a.frame_offsets =
            b.read_array<u32>(std::size_t{at} + 0x2c, a.frame_count, "scene actor frame offsets");
        a.positions = b.read_array<std::array<f32, 4>>(
            static_cast<std::size_t>(a.track_offset), a.frame_count, "scene actor position track"
        );
        c.actors.push_back(std::move(a));
    }

    // Chunks with a subtitle table start it past the fixed part of the chunk.
    if (c.header.subtitle_offset >= kSubtitleMinOffset) {
        const auto base = static_cast<std::size_t>(c.header.subtitle_offset);

        // Read 16-byte subtitle records until the end marker; the loop exits from inside.
        for (std::size_t at = base;; at += 16) {
            const s16 start = b.s16_at(at);

            // A start of -1 marks the end of the subtitles.
            if (start == -1) {
                break;
            }

            b.check(at, 16, "scene subtitle entry");
            Subtitle s;
            s.start = start;
            s.end = b.s16_at(at + 2);

            // Each subtitle has one text offset per language.
            for (std::size_t k = 0; k < kSubtitleLanguages; ++k) {
                s.text_offsets[k] = b.s16_at(at + 4 + 2 * k);
            }

            s.pad = b.s16_at(at + 14);

            // Resolve each language's text from its offset.
            for (std::size_t k = 0; k < kSubtitleLanguages; ++k) {
                const s16 o = s.text_offsets[k];

                // A negative offset means no text in this language.
                if (o < 0) {
                    continue;
                }

                const ByteView t =
                    b.tail(base + static_cast<std::size_t>(o), "scene subtitle text");
                const void* nul = std::memchr(t.data(), 0, t.size());

                // Subtitle text ends at a NUL; without one it would run past the chunk.
                if (!nul) {
                    fail("scene subtitle text without a NUL");
                }

                s.text[k].assign(
                    reinterpret_cast<const char*>(t.data()), static_cast<const u8*>(nul) - t.data()
                );
            }

            c.subtitles.push_back(std::move(s));
        }
    }

    return c;
}

Scene Scene::load(
    const LevelHeader& header, ByteView region_file, std::size_t k, SceneRegion region
) {
    // A scene index past the table has no record.
    if (k >= header.scenes.size()) {
        fail("no scene record {}", k);
    }

    const SceneEntry entry = SceneEntry::from_record(header.scenes[k]);
    const auto& table = entry.region(region);

    // The scene must have chunks in the region asked for.
    if (!table) {
        fail("scene {} has no {} chunks", k, scene_region_name(region));
    }

    // The region file must be large enough for every chunk the table names.
    if (region_file.size() < table->file_bytes()) {
        fail(
            "scene {} {}: the region file is {} bytes, the table needs {}",
            k,
            scene_region_name(region),
            region_file.size(),
            table->file_bytes()
        );
    }

    std::vector<SceneChunk> chunks;

    // Decompress each chunk from its place in the region file.
    for (std::size_t i = 0; i < table->chunks.size(); ++i) {
        const auto [at, size] = *table->chunk_in_file(i);
        chunks.push_back(parse_scene_chunk(wad_decompress(region_file.sub(at, size)), i, region));
    }

    return from_chunks(k, region, std::move(chunks));
}

Scene Scene::from_chunks(std::size_t index, SceneRegion region, std::vector<SceneChunk> chunks) {
    // A scene needs at least one chunk.
    if (chunks.empty()) {
        fail("scene {}: no chunks", index);
    }

    const s32 end = chunks.front().header.end_tick;
    const s32 tpc = static_cast<s32>(ticks_per_chunk(region));

    // All chunks carry the scene's end tick.
    for (const SceneChunk& c : chunks) {
        // A chunk that disagrees means the chunks do not belong together.
        if (c.header.end_tick != end) {
            fail("scene {}: the chunks disagree on the end tick", index);
        }
    }

    // The number of chunks must be what the end tick needs, at one fixed number of ticks per chunk.
    if ((end + tpc - 1) / tpc != static_cast<s32>(chunks.size())) {
        fail("scene {}: {} chunks for end tick {}", index, chunks.size(), end);
    }

    return {index, region, std::move(chunks)};
}

Scene Scene::from_lump(ByteView lump, std::size_t index, SceneRegion region) {
    std::vector<SceneChunk> chunks;
    const auto parts = lump_chunks(lump);

    // Decompress each part of the lump as a chunk.
    for (std::size_t i = 0; i < parts.size(); ++i) {
        chunks.push_back(parse_scene_chunk(wad_decompress(parts[i]), i, region));
    }

    return from_chunks(index, region, std::move(chunks));
}

std::vector<s32> Scene::actor_classes() const {
    std::vector<s32> out;

    // The first chunk lists every actor; later chunks carry the same ones.
    for (const SceneActor& a : chunks.front().actors) {
        out.push_back(a.klass);
    }

    return out;
}

std::pair<std::size_t, u32> Scene::chunk_at(s32 tick) const {
    const s32 tpc = static_cast<s32>(ticks_per_chunk(region));
    const s32 t = std::max(tick, 0);
    return {static_cast<std::size_t>(t / tpc), static_cast<u32>(t % tpc)};
}

const CamRecord* Scene::camera_at(s32 tick) const {
    const auto [c, r] = chunk_at(tick);

    // A tick past the last chunk's cameras has no camera record.
    if (c >= chunks.size() || r >= chunks[c].camera.size()) {
        return nullptr;
    }

    return &chunks[c].camera[r];
}

std::vector<s32> Scene::cut_ticks() const {
    std::vector<s32> out;

    // Check every tick after the first for a camera cut.
    for (s32 t = 1; t < end_tick(); ++t) {
        const CamRecord* r = camera_at(t);

        // Only cut records start a new shot.
        if (r && r->is_cut()) {
            out.push_back(t);
        }
    }

    return out;
}

const Subtitle* Scene::subtitle_at(s32 tick) const {
    const auto [c, r] = chunk_at(tick);

    // A tick past the last chunk has no subtitle.
    if (c >= chunks.size()) {
        return nullptr;
    }

    // Look through the chunk's subtitles for one on screen at this tick.
    for (const Subtitle& s : chunks[c].subtitles) {
        // At most one subtitle covers a tick, so the first match is it.
        if (s.covers(tick)) {
            return &s;
        }
    }

    return nullptr;
}

}  // namespace openrac::assets::disc
