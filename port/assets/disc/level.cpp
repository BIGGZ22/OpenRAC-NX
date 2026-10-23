// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/level.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The core index's tables and the block boundary rule.

#include "assets/disc/level.h"

#include <set>

namespace openrac::assets::disc {

namespace {

template <typename T>
std::vector<T> table(ByteView index, ArrayRange r, std::string_view what) {
    // A table with no entries or no offset is absent from this level.
    if (r.count <= 0 || r.offset <= 0) {
        return {};
    }

    // A count beyond 100000 cannot be real; stop before allocating.
    if (r.count > 100000) {
        fail("implausible count {} for {}", r.count, what);
    }

    return index
        .read_array<T>(static_cast<std::size_t>(r.offset), static_cast<std::size_t>(r.count), what);
}

}  // namespace

LevelDataHeader parse_level_data_header(ByteView data) {
    return data.read<LevelDataHeader>(0, "level data header");
}

std::optional<ByteView> LevelCore::block(ByteView data, std::string_view name) const {
    // Look the block up by name; a level has a few hundred blocks.
    for (const CoreBlock& b : blocks) {
        // Names are unique, so the first match is the block.
        if (b.name == name) {
            // A block that runs past the data is not usable.
            if (b.offset > data.size() || b.size > data.size() - b.offset) {
                return std::nullopt;
            }

            return data.sub(b.offset, b.size);
        }
    }

    return std::nullopt;
}

LevelCore parse_level_core(ByteView index, std::size_t data_size) {
    LevelCore core{};
    const auto h = index.read<LevelCoreHeader>(0, "core header");
    core.header = h;

    // Levels without gadgets have a zero count or offset.
    if (h.gadget_count > 0 && h.gadget_offset > 0) {
        core.gadgets = index.read_array<GadgetEntry>(
            static_cast<std::size_t>(h.gadget_offset),
            static_cast<std::size_t>(h.gadget_count),
            "gadget table"
        );
    }

    // A zero offset means the level has no ratchet sequence table.
    if (h.ratchet_seqs > 0) {
        core.ratchet_seqs = index.read_array<s32>(
            static_cast<std::size_t>(h.ratchet_seqs), 256, "ratchet seq table"
        );
    }

    core.gs_ram = table<GsRamEntry>(index, h.gs_ram, "gs_ram table");
    core.moby_classes = table<ClassEntry>(index, h.moby_classes, "moby class table");
    core.tie_classes = table<ClassEntry>(index, h.tie_classes, "tie class table");
    core.shrub_classes = table<ShrubClassEntry>(index, h.shrub_classes, "shrub class table");
    core.tfrag_textures = table<TextureEntry>(index, h.tfrag_textures, "tfrag textures");
    core.moby_textures = table<TextureEntry>(index, h.moby_textures, "moby textures");
    core.tie_textures = table<TextureEntry>(index, h.tie_textures, "tie textures");
    core.shrub_textures = table<TextureEntry>(index, h.shrub_textures, "shrub textures");

    // The boundary rule (ReRAC's spec 2.5): a block runs to the next known
    // start. moby_sound_remap_offset is 0x9c00 in every RAC1 level and is not
    // a data offset here.
    std::set<std::size_t> bounds;
    auto add = [&](s32 v) {
        // Only positive offsets point at data; zero marks an absent section.
        if (v > 0) {
            bounds.insert(static_cast<std::size_t>(v));
        }
    };

    // Every header offset into the data container is a boundary where some block ends.
    for (const s32 v :
         {h.tfrags,
          h.occlusion,
          h.sky,
          h.collision,
          h.textures_base_offset,
          h.part_bank_offset,
          h.fx_bank_offset}) {
        add(v);
    }

    // Each moby class starts a block, so its offset is a boundary.
    for (const ClassEntry& e : core.moby_classes) {
        add(e.offset_in_asset_wad);
    }

    // Each tie class starts a block, so its offset is a boundary.
    for (const ClassEntry& e : core.tie_classes) {
        add(e.offset_in_asset_wad);
    }

    // Each shrub class starts a block, so its offset is a boundary.
    for (const ShrubClassEntry& e : core.shrub_classes) {
        add(e.base.offset_in_asset_wad);
    }

    // Each ratchet sequence starts a block, so its offset is a boundary.
    for (const s32 v : core.ratchet_seqs) {
        add(v);
    }

    // Each gadget starts a block, so its offset is a boundary.
    for (const GadgetEntry& g : core.gadgets) {
        add(g.offset_in_asset_wad);
    }

    const std::size_t end = h.assets_decompressed_size > 0
                                ? static_cast<std::size_t>(h.assets_decompressed_size)
                                : data_size;
    bounds.insert(end);

    auto block = [&](std::string name, s32 start) {
        // A zero or negative start means the section is absent.
        if (start <= 0) {
            return;
        }

        const auto s = static_cast<std::size_t>(start);
        const auto next = bounds.upper_bound(s);
        const std::size_t stop = next == bounds.end() ? end : *next;
        core.blocks.push_back({std::move(name), s, stop >= s ? stop - s : 0});
    };
    // The tfrags open the data and run to the first of occlusion, sky, collision.
    const s32 tf_end = h.occlusion > 0 ? h.occlusion : h.sky > 0 ? h.sky : h.collision;

    // The tfrags run to the next section; an empty or inverted range has no block.
    if (tf_end > h.tfrags) {
        core.blocks.push_back(
            {"tfrags",
             static_cast<std::size_t>(h.tfrags),
             static_cast<std::size_t>(tf_end - h.tfrags)}
        );
    }

    block("occlusion", h.occlusion);
    block("sky", h.sky);
    block("collision", h.collision);
    block("textures", h.textures_base_offset);
    block("part_bank", h.part_bank_offset);
    block("fx_bank", h.fx_bank_offset);

    // One block per moby class, running to the next boundary.
    for (const ClassEntry& e : core.moby_classes) {
        block(std::format("moby_class/{:04}", e.o_class), e.offset_in_asset_wad);
    }

    // One block per tie class, running to the next boundary.
    for (const ClassEntry& e : core.tie_classes) {
        block(std::format("tie_class/{:04}", e.o_class), e.offset_in_asset_wad);
    }

    // One block per shrub class, running to the next boundary.
    for (const ShrubClassEntry& e : core.shrub_classes) {
        block(std::format("shrub_class/{:04}", e.base.o_class), e.base.offset_in_asset_wad);
    }

    // One block per ratchet sequence, running to the next boundary.
    for (std::size_t i = 0; i < core.ratchet_seqs.size(); ++i) {
        block(std::format("ratchet_seq/{:03}", i), core.ratchet_seqs[i]);
    }

    // One block per gadget, running to the next boundary.
    for (const GadgetEntry& g : core.gadgets) {
        block(std::format("gadget/{:04}", g.class_number), g.offset_in_asset_wad);
    }

    return core;
}

std::optional<HeightGrid> HeightGrid::parse(const LevelCoreHeader& header, ByteView data) {
    // A level without a height map has a zero offset.
    if (header.heightmap_offset <= 0) {
        return std::nullopt;
    }

    const auto o = static_cast<std::size_t>(header.heightmap_offset);

    // The 16-byte grid header must fit in the data.
    if (o > data.size() || data.size() - o < 16) {
        return std::nullopt;
    }

    HeightGrid g;
    g.width = data.s32_at(o);
    g.rows = data.s32_at(o + 4);
    g.low = data.f32_at(o + 8);
    g.high = data.f32_at(o + 12);

    // Negative dimensions are a damaged header.
    if (g.width < 0 || g.rows < 0) {
        return std::nullopt;
    }

    const u64 n = u64(static_cast<u32>(g.width)) * u64(static_cast<u32>(g.rows));

    // The cells must fit in the data after the header.
    if (n > data.size() - o - 16) {
        return std::nullopt;
    }

    g.cells = data.sub(o + 16, static_cast<std::size_t>(n)).to_vector();
    return g;
}

std::optional<f32> HeightGrid::height(f32 x, f32 y) const {
    const s64 i = s64{width} * static_cast<s64>(static_cast<s32>(y)) + static_cast<s32>(x);

    // A cell outside the grid has no height.
    if (i < 0 || static_cast<u64>(i) >= cells.size()) {
        return std::nullopt;
    }

    const f32 c = cells[static_cast<std::size_t>(i)];
    return (high - low) * (1.0f - c / 255.0f) + low;
}

}  // namespace openrac::assets::disc
