// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/gameplay.rs
// and crates/rc-formats/src/sound_bank.rs: ISC License, Copyright (c) 2026 ReRAC
// contributors.
//
// The gameplay file's section table and the sections the world needs.

#include "assets/world/gameplay.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "assets/world/known_games.h"

namespace openrac::assets {

namespace {

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

std::size_t pointer_of(GameplaySection section) {
    return 4 * static_cast<std::size_t>(section);
}

std::size_t index_of(s32 value, const char* what) {
    if (value < 0) {
        fail("gameplay: negative {} {}", what, value);
    }

    return static_cast<std::size_t>(value);
}

}  // namespace

std::string_view gameplay_section_name(GameplaySection section) {
    return kSectionNames.at(static_cast<std::size_t>(section));
}

GameplayFile::GameplayFile(Game game, ByteView bytes) : m_game(game), m_bytes(bytes) {
    require_world_layout(game, "gameplay file");
    bytes.check(0, 4 * kGameplaySectionCount, "gameplay header");
}

u32 GameplayFile::offset(GameplaySection section) const {
    return m_bytes.u32_at(pointer_of(section));
}

std::vector<GameplaySectionRange> GameplayFile::sections() const {
    std::vector<GameplaySectionRange> out;

    for (std::size_t i = 0; i < kGameplaySectionCount; ++i) {
        const s32 at = m_bytes.s32_at(4 * i);

        if (at > 0) {
            out.push_back({static_cast<GameplaySection>(i), static_cast<std::size_t>(at), 0});
        }
    }

    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
        return a.start < b.start;
    });

    for (std::size_t k = 0; k < out.size(); ++k) {
        out[k].end = k + 1 < out.size() ? out[k + 1].start : m_bytes.size();

        if (out[k].start > m_bytes.size() || out[k].end > m_bytes.size()) {
            fail("gameplay section {} runs past the end", gameplay_section_name(out[k].section));
        }
    }

    return out;
}

std::optional<ByteView> GameplayFile::section(GameplaySection section) const {
    for (const GameplaySectionRange& r : sections()) {
        if (r.section == section) {
            return m_bytes.sub(r.start, r.end - r.start);
        }
    }

    return std::nullopt;
}

LevelSettings read_level_settings(const GameplayFile& file) {
    const ByteView g = file.bytes();
    const std::size_t s = file.offset(GameplaySection::LevelSettings);

    if (s == 0) {
        fail("gameplay: no level settings");
    }

    g.check(s, 0x50, "level settings");
    LevelSettings l;

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

std::array<u8, 3> MobyInstance::ambient_rgb() const {
    // s32 arithmetic as the loader's, so a channel above 255 would carry.
    const u32 word = static_cast<u32>(colour[2]) * 0x10000 + static_cast<u32>(colour[1]) * 0x100
                     + static_cast<u32>(colour[0]);
    return {static_cast<u8>(word), static_cast<u8>(word >> 8), static_cast<u8>(word >> 16)};
}

std::vector<MobyInstance> read_moby_instances(const GameplayFile& file) {
    const ByteView g = file.bytes();
    const std::size_t s = file.offset(GameplaySection::MobyInstances);

    if (s == 0) {
        return {};
    }

    const s32 count = g.s32_at(s);

    if (count < 0) {
        fail("gameplay: negative moby instance count {}", count);
    }

    g.check(s + 0x10, static_cast<std::size_t>(count) * kMobyInstanceSize, "moby instances");
    std::vector<MobyInstance> out;
    out.reserve(static_cast<std::size_t>(count));

    for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
        const std::size_t r = s + 0x10 + i * kMobyInstanceSize;
        MobyInstance m;
        m.size = g.s32_at(r);

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

std::vector<s32> read_class_list(const GameplayFile& file, GameplaySection section) {
    if (section != GameplaySection::MobyClasses && section != GameplaySection::TieClasses
        && section != GameplaySection::ShrubClasses) {
        fail("gameplay: {} is not a class list", gameplay_section_name(section));
    }

    const ByteView g = file.bytes();
    const std::size_t s = file.offset(section);

    if (s == 0) {
        return {};
    }

    const std::size_t count = index_of(g.s32_at(s), "class count");
    return g.read_array<s32>(s + 0x10, count, "class list");
}

namespace {

std::vector<PvarFixup> read_fixup_list(ByteView g, std::size_t at) {
    std::vector<PvarFixup> out;

    if (at == 0) {
        return out;
    }

    for (;; at += 8) {
        const s32 index = g.s32_at(at);

        if (index < 0) {
            return out;
        }

        out.push_back({index, g.s32_at(at + 4)});
    }
}

}  // namespace

PvarFixups read_pvar_fixups(const GameplayFile& file) {
    return {
        read_fixup_list(file.bytes(), file.offset(GameplaySection::PvarMobyLinks)),
        read_fixup_list(file.bytes(), file.offset(GameplaySection::PvarPointerFixups)),
    };
}

std::vector<u8> read_pvar_shared_data(const GameplayFile& file) {
    const std::size_t s = file.offset(GameplaySection::SharedData);

    if (s == 0) {
        return {};
    }

    const std::size_t size = index_of(file.bytes().s32_at(s), "shared data size");
    return file.bytes().sub(s + 0x10, size, "pvar shared data").to_vector();
}

std::vector<PvarSharedRecord> read_pvar_shared_records(const GameplayFile& file) {
    const ByteView g = file.bytes();
    const std::size_t s = file.offset(GameplaySection::SharedData);

    if (s == 0) {
        return {};
    }

    const std::size_t size = index_of(g.s32_at(s), "shared data size");
    const std::size_t count = index_of(g.s32_at(s + 4), "shared data count");
    const std::size_t at = s + 0x10 + size;
    g.check(at, count * 8, "pvar shared records");
    std::vector<PvarSharedRecord> out;

    for (std::size_t k = 0; k < count; ++k) {
        out.push_back({g.u16_at(at + 8 * k), g.u16_at(at + 8 * k + 2), g.s32_at(at + 8 * k + 4)});
    }

    return out;
}

std::optional<std::vector<u8>> read_pvar_block(const GameplayFile& file, s32 index) {
    const ByteView g = file.bytes();
    const std::size_t table = file.offset(GameplaySection::PvarTable);
    const std::size_t data = file.offset(GameplaySection::PvarData);

    if (index < 0 || table == 0 || data == 0) {
        return std::nullopt;
    }

    const std::size_t entry = table + 8 * static_cast<std::size_t>(index);
    const s32 offset = g.s32_at(entry);
    const s32 size = g.s32_at(entry + 4);

    if (offset < 0 || size < 0) {
        fail("pvar {}: negative table entry", index);
    }

    return g
        .sub(data + static_cast<std::size_t>(offset), static_cast<std::size_t>(size), "pvar block")
        .to_vector();
}

std::vector<std::optional<std::vector<u8>>> read_pvars(
    const GameplayFile& file, std::span<const bool> spawned
) {
    const ByteView g = file.bytes();
    const std::size_t table = file.offset(GameplaySection::PvarTable);
    const std::size_t data = file.offset(GameplaySection::PvarData);

    if (table == 0 || data == 0) {
        return {};
    }

    const std::vector<MobyInstance> instances = read_moby_instances(file);
    const PvarFixups fixups = read_pvar_fixups(file);
    s32 highest = -1;

    for (const MobyInstance& m : instances) {
        highest = std::max(highest, m.pvar_index);
    }

    for (const auto* list : {&fixups.moby_links, &fixups.pointers}) {
        for (const PvarFixup& f : *list) {
            highest = std::max(highest, f.pvar_index);
        }
    }

    std::vector<std::optional<std::vector<u8>>> blocks;

    for (s32 i = 0; i <= highest; ++i) {
        const std::size_t entry = table + 8 * static_cast<std::size_t>(i);
        std::optional<std::vector<u8>> block;

        if (entry + 8 <= g.size()) {
            const s32 offset = g.s32_at(entry);
            const s32 size = g.s32_at(entry + 4);
            const std::size_t start = data + static_cast<std::size_t>(offset);

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

    for (std::size_t i = 0; i < instances.size(); ++i) {
        const bool created = spawned.empty() || i >= spawned.size() || spawned[i];
        runtime.push_back(created ? next++ : -1);
    }

    // The block and field a fixup names, when the block exists.
    auto field = [&](s32 index, s32 offset) -> u8* {
        if (index < 0 || static_cast<std::size_t>(index) >= blocks.size()
            || !blocks[static_cast<std::size_t>(index)]) {
            return nullptr;
        }

        auto& block = *blocks[static_cast<std::size_t>(index)];

        if (offset < 0) {
            fail("pvar {}: negative fixup offset {}", index, offset);
        }

        ByteView(block).check(static_cast<std::size_t>(offset), 4, "pvar fixup field");
        return block.data() + offset;
    };

    for (const PvarFixup& f : fixups.moby_links) {
        u8* p = field(f.pvar_index, f.offset);

        if (!p) {
            continue;
        }

        s32 value;
        std::memcpy(&value, p, 4);

        if (value >= 0) {
            const auto i = static_cast<std::size_t>(value);
            const s32 r = i < runtime.size() ? runtime[i] : -1;
            std::memcpy(p, &r, 4);
        }
    }

    for (const PvarFixup& f : fixups.pointers) {
        field(f.pvar_index, f.offset);  // checked only: kept block-relative
    }

    for (const PvarSharedRecord& r : read_pvar_shared_records(file)) {
        if (u8* p = field(r.pvar_index, r.offset)) {
            std::memcpy(p, &r.data_offset, 4);
        }
    }

    return blocks;
}

std::array<u8, 3> EnvTransition::fog_rgb(std::size_t side_index) const {
    const u32 c = fog_colour.at(side_index);
    return {static_cast<u8>(c), static_cast<u8>(c >> 8), static_cast<u8>(c >> 16)};
}

EnvTransitions read_env_transitions(const GameplayFile& file) {
    const ByteView g = file.bytes();
    const std::size_t s = file.offset(GameplaySection::EnvTransitions);

    if (s == 0) {
        return {};
    }

    const std::size_t count = index_of(g.s32_at(s), "env transition count");
    EnvTransitions out;
    out.circles = g.read_array<std::array<float, 4>>(s + 0x10, count, "env transition circles");
    const std::size_t records = s + 0x10 + 16 * count;
    g.check(records, count * 0x80, "env transitions");

    for (std::size_t i = 0; i < count; ++i) {
        const std::size_t r = records + 0x80 * i;
        EnvTransition z;

        for (std::size_t row = 0; row < 4; ++row) {
            for (std::size_t k = 0; k < 4; ++k) {
                z.inverse[row][k] = g.f32_at(r + 16 * row + 4 * k);
            }
        }

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

GameplayPointLights read_point_lights(const GameplayFile& file) {
    GameplayPointLights out;

    if (const auto lights = file.section(GameplaySection::PointLights)) {
        const s32 count = lights->s32_at(0);

        for (std::size_t k = 0; k < static_cast<std::size_t>(std::max(count, 0)); ++k) {
            const std::size_t at = 0x10 + 0x20 * k;
            GameplayPointLight light;

            for (std::size_t j = 0; j < 4; ++j) {
                light.position[j] = lights->f32_at(at + 4 * j);
            }

            light.rgba = lights->u32_at(at + 0x10);
            out.lights.push_back(light);
        }
    }

    if (const auto grid = file.section(GameplaySection::PointLightGrid)) {
        out.grid = grid->read_array<u32>(0, grid->size() / 4, "point light grid");
    }

    return out;
}

}  // namespace openrac::assets
