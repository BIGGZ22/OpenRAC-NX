// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/sound_bank.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// RAC1's sound definitions, remap and sound sections (level_sounds_rac1.h).

#include "assets/sound/level_sounds_rac1.h"

#include <algorithm>

namespace openrac::assets {

namespace {

constexpr std::size_t kSoundInstanceSize = 0x90;
constexpr std::size_t kEnvSamplePointSize = 0x30;
constexpr std::size_t kParkedGadgetIds = 15;

// An index-relative offset of the remap block.
std::size_t remap_relative(std::size_t base, s16 offset) {
    // A negative offset would point before the remap block.
    if (offset < 0) {
        fail("negative sound remap offset {}", offset);
    }

    return base + static_cast<std::size_t>(offset);
}

ClassSounds* find_class(LevelSounds& sounds, s32 o_class) {
    // Look the class up by its class number.
    for (ClassSounds& c : sounds.classes) {
        // Class numbers are unique, so the first match is the class.
        if (c.o_class == o_class) {
            return &c;
        }
    }

    return nullptr;
}

const ClassSounds* find_class(const LevelSounds& sounds, s32 o_class) {
    // Look the class up by its class number.
    for (const ClassSounds& c : sounds.classes) {
        // Class numbers are unique, so the first match is the class.
        if (c.o_class == o_class) {
            return &c;
        }
    }

    return nullptr;
}

// A blob's sound defs: header +0x0d count, +0x28 pointer (blob-relative).
std::vector<SoundDef> blob_defs(ByteView data, std::size_t blob, u8& count) {
    count = data.u8_at(blob + 0x0d);
    const s32 pointer = data.s32_at(blob + 0x28);

    // A class with no sounds has a zero count or no pointer.
    if (count == 0 || pointer <= 0) {
        return {};
    }

    return data
        .read_array<SoundDef>(blob + static_cast<std::size_t>(pointer), count, "class sound defs");
}

}  // namespace

LevelSounds parse_level_sounds_rac1(
    ByteView core_index,
    s32 remap_offset,
    std::span<const SoundRemapClass> classes,
    ByteView core_data
) {
    LevelSounds out;

    // A level with no remap block has no sound definitions.
    if (remap_offset <= 0) {
        return out;
    }

    const auto base = static_cast<std::size_t>(remap_offset);
    const s16 defs_offset = core_index.s16_at(base);
    const s16 defs_count = core_index.s16_at(base + 2);
    const s16 map_offset = core_index.s16_at(base + 4);
    const s16 map_count = core_index.s16_at(base + 6);

    // Negative counts are a damaged block.
    if (defs_count < 0 || map_count < 0) {
        fail("negative sound remap count");
    }

    const std::size_t map_at = remap_relative(base, map_offset);

    // Read the level's remap table: one bank index per entry.
    for (std::size_t k = 0; k < static_cast<std::size_t>(map_count); ++k) {
        out.map.push_back(core_index.u16_at(map_at + 4 * k));
    }

    out.level_defs = core_index.read_array<SoundDef>(
        remap_relative(base, defs_offset), static_cast<std::size_t>(defs_count), "level sound defs"
    );

    // A signed compare, as the loader's: an index at or past the map's end
    // becomes 0xffff; a negative one (-1 on RAC1 levels 0, 11 and 14) passes
    // and reads the u16 before the map, as the game does.
    for (SoundDef& d : out.level_defs) {
        const auto i = static_cast<s16>(d.index);

        // An index beyond the map has no remap entry and keeps its value.
        if (i < map_count) {
            const s64 at = static_cast<s64>(base) + map_offset + 4 * static_cast<s64>(i);

            // A negative address would read before the core index.
            if (at < 0) {
                fail("sound def index {} reads before the core index", i);
            }

            d.index = core_index.u16_at(static_cast<std::size_t>(at));
        } else {
            d.index = 0xffff;
        }
    }

    out.classes.reserve(classes.size());

    // Read each class's remap offset and its list of bank ids.
    for (std::size_t c = 0; c < classes.size(); ++c) {
        const std::size_t at = base + 8 + 4 * c;
        const s16 offset = core_index.s16_at(at);
        const s16 count = core_index.s16_at(at + 2);
        ClassSounds cs;
        cs.o_class = classes[c].o_class;

        // A class with no remapped sounds has a zero count.
        if (count > 0) {
            const std::size_t list = remap_relative(base, offset);

            // Read one bank id per remapped sound.
            for (std::size_t j = 0; j < static_cast<std::size_t>(count); ++j) {
                cs.bank_ids.push_back(core_index.u16_at(list + 4 * j));
            }
        }

        // Only classes with a blob have sound definitions of their own.
        if (classes[c].blob_offset > 0) {
            u8 header_count = 0;
            cs.defs = blob_defs(
                core_data, static_cast<std::size_t>(classes[c].blob_offset), header_count
            );

            // The loader writes entry j into def j for every j below the
            // header's count, walking past the class's own remap list when the
            // header count is larger (RAC1 level 10 class 1229: 12 defs, 6
            // ids; it picks up the next class's ids). The same here.
            if (!cs.defs.empty()) {
                const std::size_t list = remap_relative(base, offset);

                // Resolve each definition's index through the class's list.
                for (std::size_t j = 0; j < cs.defs.size(); ++j) {
                    cs.defs[j].index = core_index.u16_at(list + 4 * j);
                }
            }

            cs.header_count = header_count;
        }

        out.classes.push_back(std::move(cs));
    }

    return out;
}

void apply_gadget_sounds_rac1(LevelSounds& sounds, std::span<const GadgetBlob> gadgets) {
    // Gadget blobs carry definitions for classes the level index did not describe.
    for (const GadgetBlob& g : gadgets) {
        ClassSounds* c = find_class(sounds, g.o_class);

        // Skip classes the level index does not know and classes that already have their own
        // header.
        if (c == nullptr || c->header_count.has_value()) {
            continue;
        }

        u8 count = 0;
        std::vector<SoundDef> defs = blob_defs(g.blob, 0, count);
        const std::size_t parked = std::min(c->bank_ids.size(), kParkedGadgetIds);

        // Use the bank ids parked on the class as the gadget's sound indices.
        for (std::size_t j = 0; j < std::min(parked, defs.size()); ++j) {
            const u16 id = c->bank_ids[j];

            // A bank id with the sign bit set marks an unused slot.
            if (static_cast<s16>(id) >= 0) {
                defs[j].index = id;
            }
        }

        c->defs = std::move(defs);
        c->header_count = count;
    }
}

std::optional<u16> resolve_sound(const LevelSounds& sounds, SoundOwner owner, std::size_t local) {
    u16 id = 0xffff;

    // A level-owned sound is looked up in the level's own definitions.
    if (owner.level) {
        // A local index beyond the level's definitions has no sound.
        if (local >= sounds.level_defs.size()) {
            return std::nullopt;
        }

        id = sounds.level_defs[local].index;
    } else {
        const ClassSounds* c = find_class(sounds, owner.o_class);

        // A class-owned sound needs the class and an index inside its bank ids.
        if (c == nullptr || local >= c->bank_ids.size()) {
            return std::nullopt;
        }

        id = c->bank_ids[local];
    }

    // The id 0xffff marks an unused slot.
    if (id == 0xffff) {
        return std::nullopt;
    }

    return id;
}

const SoundDef* find_sound_def(const LevelSounds& sounds, SoundOwner owner, std::size_t local) {
    // A level-owned sound is in the level's definitions; a class-owned one is in its class.
    if (owner.level) {
        return local < sounds.level_defs.size() ? &sounds.level_defs[local] : nullptr;
    }

    const ClassSounds* c = find_class(sounds, owner.o_class);

    // A class that is unknown or has too few definitions has no sound for this index.
    if (c == nullptr || local >= c->defs.size()) {
        return nullptr;
    }

    return &c->defs[local];
}

std::array<f32, 3> SoundInstance::to_local(std::array<f32, 3> v) const {
    std::array<f32, 3> out{};

    // Multiply the vector by the inverse matrix's columns to take it into the instance's local
    // frame.
    for (std::size_t k = 0; k < 3; ++k) {
        out[k] = v[0] * inverse[0][k] + v[1] * inverse[1][k] + v[2] * inverse[2][k];
    }

    return out;
}

std::vector<SoundInstance> parse_sound_instances_rac1(ByteView gameplay) {
    const std::size_t p = gameplay.u32_at(kSoundInstancesPointerRac1);

    // A zero pointer means the level has no sound instances.
    if (p == 0) {
        return {};
    }

    const s32 count = gameplay.s32_at(p);

    // A negative count is a damaged section.
    if (count < 0) {
        fail("negative sound instance count");
    }

    const auto n = static_cast<std::size_t>(count);
    gameplay.check(p + 0x10, kSoundInstanceSize * n, "sound instances");
    std::vector<SoundInstance> out(n);

    // Read each instance's record: range, matrices and rotation.
    for (std::size_t i = 0; i < n; ++i) {
        const std::size_t o = p + 0x10 + kSoundInstanceSize * i;
        SoundInstance& s = out[i];
        s.o_class = gameplay.s16_at(o);
        s.m_class = gameplay.s16_at(o + 2);
        s.pvar_index = gameplay.s32_at(o + 8);
        s.range = gameplay.f32_at(o + 0x0c);

        // The instance's 4x4 matrix, row by row.
        for (std::size_t r = 0; r < 4; ++r) {
            // Each row holds four floats.
            for (std::size_t c = 0; c < 4; ++c) {
                s.matrix[r][c] = gameplay.f32_at(o + 0x10 + 16 * r + 4 * c);
            }
        }

        // The instance's inverse matrix keeps only three rows, as the fourth is implied.
        for (std::size_t r = 0; r < 3; ++r) {
            // Each row holds four floats.
            for (std::size_t c = 0; c < 4; ++c) {
                s.inverse[r][c] = gameplay.f32_at(o + 0x50 + 16 * r + 4 * c);
            }
        }

        // The instance's three rotation angles.
        for (std::size_t k = 0; k < 3; ++k) {
            s.rotation[k] = gameplay.f32_at(o + 0x80 + 4 * k);
        }
    }

    return out;
}

std::vector<EnvSamplePoint> parse_env_sample_points_rac1(ByteView gameplay) {
    const std::size_t p = gameplay.u32_at(kEnvSamplePointsPointerRac1);

    // A zero pointer means the level has no environment sample points.
    if (p == 0) {
        return {};
    }

    const s32 count = gameplay.s32_at(p);

    // A negative count is a damaged section.
    if (count < 0) {
        fail("negative env sample point count");
    }

    std::vector<EnvSamplePoint> out(static_cast<std::size_t>(count));

    // Read each environment sample point's record.
    for (std::size_t i = 0; i < out.size(); ++i) {
        const std::size_t o = p + 0x10 + kEnvSamplePointSize * i;
        EnvSamplePoint& e = out[i];
        e.position = {gameplay.f32_at(o), gameplay.f32_at(o + 4), gameplay.f32_at(o + 8)};
        e.reverb_depth = gameplay.s32_at(o + 0x20);
        e.reverb_type = gameplay.u8_at(o + 0x24);
        e.reverb_delay = gameplay.u8_at(o + 0x25);
        e.reverb_feedback = gameplay.u8_at(o + 0x26);
        e.reverb_enable = gameplay.u8_at(o + 0x27);
        e.music_track = gameplay.s32_at(o + 0x28);
    }

    return out;
}

}  // namespace openrac::assets
