// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/gameplay.rs
// and crates/rc-formats/src/sound_bank.rs: ISC License, Copyright (c) 2026 ReRAC
// contributors.
//
// Implements gameplay.h: the gameplay file's section table, and readers for the sections the
// world needs (level settings, moby instances, class lists, the pvar blocks and their fixups,
// environment transitions and point lights). The file starts with one offset per section; a
// section runs to the next one's start. Every reader bounds checks what it follows, and a
// missing section reads as empty unless the level cannot do without it.

#include "assets/world/gameplay.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "assets/world/known_games.h"

namespace openrac::assets {

namespace {

// The section names, in the order of the header's offsets (the GameplaySection enum).
constexpr std::array<std::string_view, kGameplaySectionCount> kSectionNames{
    "level_settings",
    "directional_lights",
    "cameras",
    "sound_instances",
    "help_us_english",
    "help_uk_english",
    "help_french",
    "help_german",
    "help_spanish",
    "help_italian",
    "help_japanese",
    "help_korean",
    "tie_classes",
    "tie_instances",
    "shrub_classes",
    "shrub_instances",
    "moby_classes",
    "moby_instances",
    "moby_groups",
    "shared_data",
    "pvar_moby_links",
    "pvar_table",
    "pvar_data",
    "pvar_pointer_fixups",
    "cuboids",
    "spheres",
    "cylinders",
    "pills",
    "paths",
    "grind_paths",
    "point_light_grid",
    "point_lights",
    "env_transitions",
    "camera_collision_grid",
    "env_sample_points",
    "occlusion_mappings",
    "unused_90",
};

// The byte position of a section's offset in the header.
std::size_t pointer_of(GameplaySection section) {
    return 4 * static_cast<std::size_t>(section);
}

// Checks that an s32 from the file is a valid count or index, and widens it. `what` names it.
std::size_t index_of(s32 value, const char* what) {
    // The file's counts and sizes are s32 but only make sense as non-negative values.
    if (value < 0) {
        fail("gameplay: negative {} {}", what, value);
    }

    return static_cast<std::size_t>(value);
}

}  // namespace

// Throws (via at()) for a value outside the enum.
std::string_view gameplay_section_name(GameplaySection section) {
    return kSectionNames.at(static_cast<std::size_t>(section));
}

// Borrows the bytes; the caller keeps them alive. Fails when the header does not fit.
GameplayFile::GameplayFile(Game game, ByteView bytes) : m_game(game), m_bytes(bytes) {
    require_world_layout(game, "gameplay file");
    bytes.check(0, 4 * kGameplaySectionCount, "gameplay header");
}

// The section's offset from the file start, 0 when the level has none.
u32 GameplayFile::offset(GameplaySection section) const {
    return m_bytes.u32_at(pointer_of(section));
}

// The sections the file has, sorted by start, each with its end.
std::vector<GameplaySectionRange> GameplayFile::sections() const {
    std::vector<GameplaySectionRange> out;

    // The header is one u32 offset per section, in section order.
    for (std::size_t i = 0; i < kGameplaySectionCount; ++i) {
        const s32 at = m_bytes.s32_at(4 * i);

        // An offset of 0 (or below) means the level has no such section.
        if (at > 0) {
            out.push_back({static_cast<GameplaySection>(i), static_cast<std::size_t>(at), 0});
        }
    }

    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
        return a.start < b.start;
    });

    // Each section runs to the start of the next one in the file, the last to the end.
    for (std::size_t k = 0; k < out.size(); ++k) {
        out[k].end = k + 1 < out.size() ? out[k + 1].start : m_bytes.size();

        // A section outside the file means the header is damaged.
        if (out[k].start > m_bytes.size() || out[k].end > m_bytes.size()) {
            fail("gameplay section {} runs past the end", gameplay_section_name(out[k].section));
        }
    }

    return out;
}

// The section's bytes, or nothing when the level has none.
std::optional<ByteView> GameplayFile::section(GameplaySection section) const {
    // Look the section up among those the file has.
    for (const GameplaySectionRange& r : sections()) {
        // This is the section asked for.
        if (r.section == section) {
            return m_bytes.sub(r.start, r.end - r.start);
        }
    }

    return std::nullopt;
}

// Fields are at fixed offsets from the section start (see gameplay.h); 0x50 bytes must fit.
LevelSettings read_level_settings(const GameplayFile& file) {
    const ByteView g = file.bytes();
    const std::size_t s = file.offset(GameplaySection::LevelSettings);

    // A zero offset means the level has no settings, which no level lacks.
    if (s == 0) {
        fail("gameplay: no level settings");
    }

    g.check(s, 0x50, "level settings");
    LevelSettings l;

    // Three colour channels and three position axes per record.
    for (std::size_t k = 0; k < 3; ++k) {
        l.background_rgb[k] = g.s32_at(s + 4 * k);
        l.fog_rgb[k] = g.s32_at(s + 0x0c + 4 * k);
        l.ship_position[k] = g.f32_at(s + 0x2c + 4 * k);
    }

    l.fog_near = g.f32_at(s + 0x18);
    l.fog_far = g.f32_at(s + 0x1c);
    l.fog_near_intensity = g.f32_at(s + 0x20);
    l.fog_far_intensity = g.f32_at(s + 0x24);
    l.death_height = g.f32_at(s + 0x28);
    l.ship_yaw = g.f32_at(s + 0x38);
    l.ship_path = g.s32_at(s + 0x3c);
    l.ship_camera_cuboid_first = g.s32_at(s + 0x40);
    l.ship_camera_cuboid_last = g.s32_at(s + 0x44);
    return l;
}

// The ambient colour as the 8-bit channels the loader packs from three s32 values.
std::array<u8, 3> MobyInstance::ambient_rgb() const {
    // s32 arithmetic as the loader's, so a channel above 255 would carry.
    const u32 word = static_cast<u32>(colour[2]) * 0x10000 + static_cast<u32>(colour[1]) * 0x100
                     + static_cast<u32>(colour[0]);
    return {static_cast<u8>(word), static_cast<u8>(word >> 8), static_cast<u8>(word >> 16)};
}

// The records follow a count and a 0x10-byte header; each is checked against its own size field.
std::vector<MobyInstance> read_moby_instances(const GameplayFile& file) {
    const ByteView g = file.bytes();
    const std::size_t s = file.offset(GameplaySection::MobyInstances);

    // A level with no moby instances has none to read.
    if (s == 0) {
        return {};
    }

    const s32 count = g.s32_at(s);

    // A negative count would size the bounds check below wrongly.
    if (count < 0) {
        fail("gameplay: negative moby instance count {}", count);
    }

    g.check(s + 0x10, static_cast<std::size_t>(count) * kMobyInstanceSize, "moby instances");
    std::vector<MobyInstance> out;
    out.reserve(static_cast<std::size_t>(count));

    // Instances follow the 0x10-byte section header, kMobyInstanceSize bytes each.
    for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
        const std::size_t r = s + 0x10 + i * kMobyInstanceSize;
        MobyInstance m;
        m.size = g.s32_at(r);

        // Every instance record carries its own size, which must match the layout read here.
        if (m.size != static_cast<s32>(kMobyInstanceSize)) {
            fail("moby instance {}: size {:#x}, expected 0x78", i, m.size);
        }

        m.mission = g.s32_at(r + 0x04);
        m.spawn_flags = g.s32_at(r + 0x08);
        m.spawn_id = g.s32_at(r + 0x0c);
        m.start_value = g.s32_at(r + 0x10);
        m.start_value_done = g.s32_at(r + 0x14);
        m.class_id = g.s32_at(r + 0x18);
        m.scale = g.f32_at(r + 0x1c);
        m.draw_distance = g.s32_at(r + 0x20);
        m.update_distance = g.s32_at(r + 0x24);
        m.unused_28 = g.s32_at(r + 0x28);
        m.unused_2c = g.s32_at(r + 0x2c);

        // Three components per vector: position, rotation and colour.
        for (std::size_t k = 0; k < 3; ++k) {
            m.position[k] = g.f32_at(r + 0x30 + 4 * k);
            m.rotation[k] = g.f32_at(r + 0x3c + 4 * k);
            m.colour[k] = g.s32_at(r + 0x64 + 4 * k);
        }

        m.group = g.s32_at(r + 0x48);
        m.is_rooted = g.s32_at(r + 0x4c);
        m.rooted_distance = g.f32_at(r + 0x50);
        m.unknown_54 = g.s32_at(r + 0x54);
        m.pvar_index = g.s32_at(r + 0x58);
        m.occlusion = g.s32_at(r + 0x5c);
        m.mode_bits = g.s32_at(r + 0x60);
        m.light = g.s32_at(r + 0x70);
        m.unknown_74 = g.s32_at(r + 0x74);
        out.push_back(m);
    }

    return out;
}

// The class ids of a moby, tie or shrub section, in file order.
std::vector<s32> read_class_list(const GameplayFile& file, GameplaySection section) {
    // Only the tie, shrub and moby sections have the class list layout.
    if (section != GameplaySection::MobyClasses && section != GameplaySection::TieClasses
        && section != GameplaySection::ShrubClasses) {
        fail("gameplay: {} is not a class list", gameplay_section_name(section));
    }

    const ByteView g = file.bytes();
    const std::size_t s = file.offset(section);

    // A level with no such section has no classes.
    if (s == 0) {
        return {};
    }

    const std::size_t count = index_of(g.s32_at(s), "class count");
    return g.read_array<s32>(s + 0x10, count, "class list");
}

namespace {

// Reads the fixup list at `at`: 8-byte entries of pvar index and field offset, up to a negative
// index. An offset of 0 means no list.
std::vector<PvarFixup> read_fixup_list(ByteView g, std::size_t at) {
    std::vector<PvarFixup> out;

    // A zero offset means the list is absent.
    if (at == 0) {
        return out;
    }

    // The list is a run of 8-byte entries, ended by a negative pvar index.
    for (;; at += 8) {
        const s32 index = g.s32_at(at);

        // The negative index is the terminator.
        if (index < 0) {
            return out;
        }

        out.push_back({index, g.s32_at(at + 4)});
    }
}

}  // namespace

// The moby link list and the pointer list, which the pvar blocks need patched.
PvarFixups read_pvar_fixups(const GameplayFile& file) {
    return {
        read_fixup_list(file.bytes(), file.offset(GameplaySection::PvarMobyLinks)),
        read_fixup_list(file.bytes(), file.offset(GameplaySection::PvarPointerFixups)),
    };
}

// The data shared by several pvar blocks: a size, then that many bytes after the header.
std::vector<u8> read_pvar_shared_data(const GameplayFile& file) {
    const std::size_t s = file.offset(GameplaySection::SharedData);

    // A level with no shared data has none to read.
    if (s == 0) {
        return {};
    }

    const std::size_t size = index_of(file.bytes().s32_at(s), "shared data size");
    return file.bytes().sub(s + 0x10, size, "pvar shared data").to_vector();
}

// The records after the shared data say where each block's field points into it.
std::vector<PvarSharedRecord> read_pvar_shared_records(const GameplayFile& file) {
    const ByteView g = file.bytes();
    const std::size_t s = file.offset(GameplaySection::SharedData);

    // A level with no shared data has no records.
    if (s == 0) {
        return {};
    }

    const std::size_t size = index_of(g.s32_at(s), "shared data size");
    const std::size_t count = index_of(g.s32_at(s + 4), "shared data count");
    const std::size_t at = s + 0x10 + size;
    g.check(at, count * 8, "pvar shared records");
    std::vector<PvarSharedRecord> out;

    // Each record is 8 bytes: two u16 and an s32.
    for (std::size_t k = 0; k < count; ++k) {
        out.push_back({g.u16_at(at + 8 * k), g.u16_at(at + 8 * k + 2), g.s32_at(at + 8 * k + 4)});
    }

    return out;
}

// One block's raw bytes, unpatched; nothing for index -1 or a level without pvars.
std::optional<std::vector<u8>> read_pvar_block(const GameplayFile& file, s32 index) {
    const ByteView g = file.bytes();
    const std::size_t table = file.offset(GameplaySection::PvarTable);
    const std::size_t data = file.offset(GameplaySection::PvarData);

    // A negative index, or a level without the pvar table or data, has no block.
    if (index < 0 || table == 0 || data == 0) {
        return std::nullopt;
    }

    const std::size_t entry = table + 8 * static_cast<std::size_t>(index);
    const s32 offset = g.s32_at(entry);
    const s32 size = g.s32_at(entry + 4);

    // A negative offset or size would address outside the data.
    if (offset < 0 || size < 0) {
        fail("pvar {}: negative table entry", index);
    }

    return g
        .sub(data + static_cast<std::size_t>(offset), static_cast<std::size_t>(size), "pvar block")
        .to_vector();
}

// All pvar blocks as the loader leaves them: moby links turned into runtime moby indices (a link
// to an uncreated instance becomes -1), and shared data offsets patched in. `spawned` says which
// instances the loader created; empty means all.
std::vector<std::optional<std::vector<u8>>> read_pvars(
    const GameplayFile& file, std::span<const bool> spawned
) {
    const ByteView g = file.bytes();
    const std::size_t table = file.offset(GameplaySection::PvarTable);
    const std::size_t data = file.offset(GameplaySection::PvarData);

    // A level without the pvar table or data has no pvars.
    if (table == 0 || data == 0) {
        return {};
    }

    const std::vector<MobyInstance> instances = read_moby_instances(file);
    const PvarFixups fixups = read_pvar_fixups(file);
    s32 highest = -1;

    // The highest pvar index any instance uses sets the number of blocks.
    for (const MobyInstance& m : instances) {
        highest = std::max(highest, m.pvar_index);
    }

    // Fixups also name pvar indices, so they count towards the highest.
    for (const auto* list : {&fixups.moby_links, &fixups.pointers}) {
        // Take each fixup of this list.
        for (const PvarFixup& f : *list) {
            highest = std::max(highest, f.pvar_index);
        }
    }

    std::vector<std::optional<std::vector<u8>>> blocks;

    // One block per index up to the highest; a block that is not in the file stays empty.
    for (s32 i = 0; i <= highest; ++i) {
        const std::size_t entry = table + 8 * static_cast<std::size_t>(i);
        std::optional<std::vector<u8>> block;

        // A table entry outside the file leaves its block empty.
        if (entry + 8 <= g.size()) {
            const s32 offset = g.s32_at(entry);
            const s32 size = g.s32_at(entry + 4);
            const std::size_t start = data + static_cast<std::size_t>(offset);

            // A block outside the data leaves its block empty rather than failing.
            if (offset >= 0 && size >= 0 && start <= g.size()
                && static_cast<std::size_t>(size) <= g.size() - start) {
                block = g.sub(start, static_cast<std::size_t>(size)).to_vector();
            }
        }

        blocks.push_back(std::move(block));
    }

    // The runtime index of instance i (0x1acc00[i]): the created instances
    // before it, or -1.
    std::vector<s32> runtime;
    s32 next = 0;

    // Number the instances the loader creates, in order: a skipped instance gets -1.
    for (std::size_t i = 0; i < instances.size(); ++i) {
        const bool created = spawned.empty() || i >= spawned.size() || spawned[i];
        runtime.push_back(created ? next++ : -1);
    }

    // The block and field a fixup names, when the block exists.
    auto field = [&](s32 index, s32 offset) -> u8* {
        // A fixup into a block that does not exist is ignored.
        if (index < 0 || static_cast<std::size_t>(index) >= blocks.size()
            || !blocks[static_cast<std::size_t>(index)]) {
            return nullptr;
        }

        auto& block = *blocks[static_cast<std::size_t>(index)];

        // A negative offset would address before the block.
        if (offset < 0) {
            fail("pvar {}: negative fixup offset {}", index, offset);
        }

        ByteView(block).check(static_cast<std::size_t>(offset), 4, "pvar fixup field");
        return block.data() + offset;
    };

    // Moby link fixups hold instance indices; turn each into the runtime moby index.
    for (const PvarFixup& f : fixups.moby_links) {
        u8* p = field(f.pvar_index, f.offset);

        // The fixup names a block that does not exist, so there is nothing to patch.
        if (!p) {
            continue;
        }

        s32 value;
        std::memcpy(&value, p, 4);

        // A negative value is the file's "no moby"; leave it.
        if (value >= 0) {
            const auto i = static_cast<std::size_t>(value);
            const s32 r = i < runtime.size() ? runtime[i] : -1;
            std::memcpy(p, &r, 4);
        }
    }

    // Pointer fixups stay block-relative here, but their offsets are still checked.
    for (const PvarFixup& f : fixups.pointers) {
        field(f.pvar_index, f.offset);  // checked only: kept block-relative
    }

    // Shared records patch a data offset into each block that uses the shared data.
    for (const PvarSharedRecord& r : read_pvar_shared_records(file)) {
        // Only blocks that exist take a patch.
        if (u8* p = field(r.pvar_index, r.offset)) {
            std::memcpy(p, &r.data_offset, 4);
        }
    }

    return blocks;
}

// The fog colour of one side, as 8-bit channels from the low three bytes.
std::array<u8, 3> EnvTransition::fog_rgb(std::size_t side_index) const {
    const u32 c = fog_colour.at(side_index);
    return {static_cast<u8>(c), static_cast<u8>(c >> 8), static_cast<u8>(c >> 16)};
}

// The circles come first, then 0x80-byte transition records; both counts are the section's.
EnvTransitions read_env_transitions(const GameplayFile& file) {
    const ByteView g = file.bytes();
    const std::size_t s = file.offset(GameplaySection::EnvTransitions);

    // A level with no env transitions has none.
    if (s == 0) {
        return {};
    }

    const std::size_t count = index_of(g.s32_at(s), "env transition count");
    EnvTransitions out;
    out.circles = g.read_array<std::array<float, 4>>(s + 0x10, count, "env transition circles");
    const std::size_t records = s + 0x10 + 16 * count;
    g.check(records, count * 0x80, "env transitions");

    // Each transition record is 0x80 bytes, after the circles.
    for (std::size_t i = 0; i < count; ++i) {
        const std::size_t r = records + 0x80 * i;
        EnvTransition z;

        // The first 64 bytes are a 4 x 4 matrix, row by row.
        for (std::size_t row = 0; row < 4; ++row) {
            // Four floats per row.
            for (std::size_t k = 0; k < 4; ++k) {
                z.inverse[row][k] = g.f32_at(r + 16 * row + 4 * k);
            }
        }

        // Two sides per transition (t = 0 and t = 1): the hero colour and light, the fog colour,
        // and four floats of side data each.
        for (std::size_t k = 0; k < 2; ++k) {
            z.hero_colour[k] = g.u32_at(r + 0x40 + 4 * k);
            z.hero_light[k] = g.s32_at(r + 0x48 + 4 * k);

            // The loader's fixup: a light below zero becomes 0xb.
            if (z.hero_light[k] < 0) {
                z.hero_light[k] = 0xb;
            }

            z.fog_colour[k] = g.u32_at(r + 0x54 + 4 * k);
            const std::size_t side = r + 0x5c + 16 * k;
            z.side[k] =
                {g.f32_at(side), g.f32_at(side + 4), g.f32_at(side + 8), g.f32_at(side + 12)};
        }

        z.flags = g.u32_at(r + 0x50);
        z.unused_7c = g.u32_at(r + 0x7c);
        out.transitions.push_back(z);
    }

    return out;
}

// The point light records and the grid that indexes them; both sections are optional.
GameplayPointLights read_point_lights(const GameplayFile& file) {
    GameplayPointLights out;

    // A level with no point light section has no lights.
    if (const auto lights = file.section(GameplaySection::PointLights)) {
        const s32 count = lights->s32_at(0);

        // Each light is 0x20 bytes after the 0x10-byte header; a negative count reads as none.
        for (std::size_t k = 0; k < static_cast<std::size_t>(std::max(count, 0)); ++k) {
            const std::size_t at = 0x10 + 0x20 * k;
            GameplayPointLight light;

            // Four floats: the position and the squared radius.
            for (std::size_t j = 0; j < 4; ++j) {
                light.position[j] = lights->f32_at(at + 4 * j);
            }

            light.rgba = lights->u32_at(at + 0x10);
            out.lights.push_back(light);
        }
    }

    // The grid is optional; when present it is a plain array of u32.
    if (const auto grid = file.section(GameplaySection::PointLightGrid)) {
        out.grid = grid->read_array<u32>(0, grid->size() / 4, "point light grid");
    }

    return out;
}

}  // namespace openrac::assets
