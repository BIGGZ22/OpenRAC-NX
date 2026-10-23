// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/gameplay.rs
// and crates/rc-formats/src/sound_bank.rs (`pvar_block`): ISC License, Copyright (c) 2026
// ReRAC contributors.
//
// A level's gameplay file (the decompressed gameplay_ntsc or gameplay_pal
// lump): what the level loader places. Specification: ReRAC's
// docs/formats/wad_layouts_rac1.md section 3; what the loader does with each
// field is from its reading of the level loader, NTSC-U level01 0x255958
// ("InitLevelRenderGlobals"), cited per field.
//
// The file starts with 37 s32 section offsets, 0 for an absent section. rac1's
// order is its own (no tie or shrub groups, a point light grid at 0x78); the
// sequels' are not known here. Sections read elsewhere: tie and shrub
// instances (the geometry readers), the directional lights (lighting), sound
// instances and environment sample points (sound), help messages (text).
// The shapes and grind paths are volumes.h; the cameras cameras.h; the
// occlusion mappings occlusion.h.

#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "assets/bytes.h"
#include "assets/version.h"

namespace openrac::assets {

// The section offsets of the header, in their order (offset = 4 * value).
enum class GameplaySection : u8 {
    LevelSettings,
    DirectionalLights,
    Cameras,
    SoundInstances,
    HelpUsEnglish,
    HelpUkEnglish,
    HelpFrench,
    HelpGerman,
    HelpSpanish,
    HelpItalian,
    HelpJapanese,
    HelpKorean,
    TieClasses,
    TieInstances,
    ShrubClasses,
    ShrubInstances,
    MobyClasses,
    MobyInstances,
    MobyGroups,
    SharedData,
    PvarMobyLinks,
    PvarTable,
    PvarData,
    PvarPointerFixups,
    Cuboids,
    Spheres,
    Cylinders,
    Pills,
    Paths,
    GrindPaths,
    PointLightGrid,
    PointLights,
    EnvTransitions,
    CameraCollisionGrid,
    EnvSamplePoints,
    OcclusionMappings,
    Unused90,
};

inline constexpr std::size_t kGameplaySectionCount = 37;

std::string_view gameplay_section_name(GameplaySection section);

struct GameplaySectionRange {
    GameplaySection section;
    std::size_t start = 0;
    std::size_t end = 0;
};

// A gameplay file and its section table. Borrows the bytes.
class GameplayFile {
public:
    GameplayFile(Game game, ByteView bytes);

    Game game() const { return m_game; }

    ByteView bytes() const { return m_bytes; }

    // The section's offset in the file, 0 when it is absent.
    u32 offset(GameplaySection section) const;

    // The present sections in file order, each running to the next section's
    // offset (the last to the end of the file).
    std::vector<GameplaySectionRange> sections() const;

    // The bytes of one section as sections() bounds it, or nothing.
    std::optional<ByteView> section(GameplaySection section) const;

private:
    Game m_game;
    ByteView m_bytes;
};

// ---- Level settings ----

// Section 0x00, 0x50 bytes in rac1.
struct LevelSettings {
    std::array<s32, 3> background_rgb{};  // 0..255; r = -1: none
    std::array<s32, 3> fog_rgb{};         // the same
    float fog_near = 0;
    float fog_far = 0;
    float fog_near_intensity = 0;
    float fog_far_intensity = 0;
    float death_height = 0;  // below it the player dies
    std::array<float, 3> ship_position{};
    float ship_yaw = 0;  // rotation about Z
    s32 ship_path = 0;   // into the paths
    s32 ship_camera_cuboid_first = 0;
    s32 ship_camera_cuboid_last = 0;

    // Where the loader creates the player's ship: when x > 0 it creates the
    // ship moby at the position with this yaw (moby+0x48); x <= 0 means the
    // level has no ship.
    bool has_ship() const { return ship_position[0] > 0.0f; }
};

LevelSettings read_level_settings(const GameplayFile& file);

// ---- Moby instances ----

// Section 0x44.
inline constexpr std::size_t kMobyInstanceSize = 0x78;

// One placed moby (0x78 bytes). "moby+X" names the field of the 0x100-byte
// runtime moby the loader copies it to.
struct MobyInstance {
    s32 size = 0;  // 0x78; the loader steps by it
    // +0x04: moby+0xb0, and the index of the level mission whose byte the
    // spawn test reads (the game decides). -1 on most.
    s32 mission = 0;
    // +0x08: spawn conditions, 0 for always (the game decides).
    s32 spawn_flags = 0;
    // +0x0c: moby+0xb2 (s16): the bit index of the spawn tests, unique per
    // instance (-2 on some); also the key of the occlusion mappings.
    s32 spawn_id = 0;
    // +0x10 / +0x14: the values moby+0xb4 / +0xb6 start from (+0x14 once the
    // mission is done; the game decides).
    s32 start_value = 0;
    s32 start_value_done = 0;
    s32 class_id = 0;  // +0x18: the moby class number
    // +0x1c: the instance scale; moby+0x2c is the class scale (class header
    // +0x24) times this.
    float scale = 0;
    // +0x20: moby+0x32 (s16). Wrench calls it an f32; the loader reads an
    // integer (64 on Novalis), which the moby renderer culls and fades by.
    s32 draw_distance = 0;
    s32 update_distance = 0;  // +0x24: moby+0x30 (byte)
    s32 unused_28 = 0;        // 32 on the disc
    s32 unused_2c = 0;        // 64 on the disc
    std::array<float, 3> position{};
    // +0x3c: Euler angles in radians (moby+0x40); the matrix is
    // Rz(z) * Ry(y) * Rx(x), x applied first.
    std::array<float, 3> rotation{};
    s32 group = 0;      // +0x48: the moby group or -1 (moby+0x21)
    s32 is_rooted = 0;  // non-zero: z = the ground below + rooted_distance
    float rooted_distance = 0;
    s32 unknown_54 = 0;
    s32 pvar_index = 0;  // +0x58: or -1
    // +0x5c: 0 lets the occlusion mappings decide (occlusion.h); otherwise the
    // moby is always visible (moby+0x36 = 0x7f80).
    s32 occlusion = 0;
    // +0x60: or'd into moby+0x34 (mode bits): 0x8000 mirrors the second
    // rotation row, 0x100 keeps the existing rows.
    s32 mode_bits = 0;
    // +0x64: the ambient colour r, g, b (0..255, 128 is 1.0), moby+0x3c.
    std::array<s32, 3> colour{};
    // +0x70: moby+0x38: byte 0 the directional light set, byte 1 a second set,
    // byte 2 the cross-fade weight between them (0 none).
    s32 light = 0;
    // +0x74: when not -1 the loader registers the moby with a hook (an NPC's
    // talk slot).
    s32 unknown_74 = 0;

    // moby+0x3c..0x3e: the low three bytes of b * 0x10000 + g * 0x100 + r.
    std::array<u8, 3> ambient_rgb() const;

    u16 mode() const { return static_cast<u16>(mode_bits); }
};

// The static instances: `s32 static_count, s32 spawnable_count, pad[2]`, then
// the records. Every record's size must be 0x78. An absent section gives none.
std::vector<MobyInstance> read_moby_instances(const GameplayFile& file);

// The class numbers of a class list section (MobyClasses, TieClasses,
// ShrubClasses): `s32 count, pad[3]`, then `count` s32.
std::vector<s32> read_class_list(const GameplayFile& file, GameplaySection section);

// ---- Pvars ----

// Per-instance class variables.
struct PvarFixup {
    s32 pvar_index = 0;
    s32 offset = 0;  // the byte offset of an s32 inside the block
};

struct PvarFixups {
    // Section 0x50: the s32 is a gameplay instance index; the loader replaces
    // it with the runtime moby index (-1 when that instance was not created).
    // Negative values are kept.
    std::vector<PvarFixup> moby_links;
    // Section 0x5c: the s32 is an offset in its own block; the loader adds the
    // block's address. Here it stays block-relative.
    std::vector<PvarFixup> pointers;
};

// Both lists end at an entry whose pvar index is negative.
PvarFixups read_pvar_fixups(const GameplayFile& file);

// Section 0x4c's data blob (`s32 size, s32 count, pad[2], data[size]`), empty
// when absent.
std::vector<u8> read_pvar_shared_data(const GameplayFile& file);

struct PvarSharedRecord {
    u16 pvar_index = 0;
    u16 offset = 0;       // the field in the block
    s32 data_offset = 0;  // into the shared data blob
};

// Section 0x4c's records, after the blob: each makes a pvar field point into
// the blob.
std::vector<PvarSharedRecord> read_pvar_shared_records(const GameplayFile& file);

// One pvar block as stored (the table's {s32 offset, s32 size} entry into the
// data section), or nothing for index -1 or an absent table.
std::optional<std::vector<u8>> read_pvar_block(const GameplayFile& file, s32 index);

// The pvar blocks as the level loader leaves them, by pvar index. `spawned[i]`
// says whether instance i was created (empty: all were; the game decides).
// After copying, the loader applies in order: the moby links (an instance
// index becomes the number of created instances before it, or -1; an index
// past the list also -1), the pointer fixups (kept block-relative here, only
// checked), and the shared records (the field takes the blob-relative offset
// here, where the game stores an address). The file has no pvar count: there
// are as many as the highest index an instance or fixup names, plus one. An
// entry outside the data is nothing.
std::vector<std::optional<std::vector<u8>>> read_pvars(
    const GameplayFile& file, std::span<const bool> spawned = {}
);

// ---- Paths ----

// Section 0x70.
// The paths (splines) are disc/volumes.h's parse_paths.

// ---- Environment transitions ----

// Section 0x80: the fog zones.
// One end of a transition: the level fog at t = 0 or t = 1.
struct EnvTransitionSide {
    float near_distance = 0;
    float near_density = 0;
    float far_distance = 0;
    float far_density = 0;
};

// One transition (0x80 bytes) as the loader copies it.
struct EnvTransition {
    // +0x00: world -> cuboid-local, row-vector form: l = x * row0 + y * row1 +
    // z * row2 + row3. The cuboid is |l.xyz| <= 1; the blend runs along local
    // x, t = (l.x + 1) / 2.
    std::array<std::array<float, 4>, 4> inverse{};
    std::array<u32, 2> hero_colour{};  // bytes r, g, b, a at t = 0 and t = 1
    // The hero's directional light bank at t = 0 and t = 1. The file has -1
    // for none, which the loader turns into 0xb.
    std::array<s32, 2> hero_light{};
    // Bit 0 cross-fades the hero's lighting, bit 1 lerps the level fog.
    u32 flags = 0;
    std::array<u32, 2> fog_colour{};  // bytes r, g, b
    std::array<EnvTransitionSide, 2> side{};
    u32 unused_7c = 0;

    bool lerps_fog() const { return (flags & 2) != 0; }

    bool lerps_hero_light() const { return (flags & 1) != 0; }

    std::array<u8, 3> fog_rgb(std::size_t side_index) const;
};

struct EnvTransitions {
    // A 2-D bounding circle in XY per transition: x, y, z (unread), r^2.
    std::vector<std::array<float, 4>> circles;
    std::vector<EnvTransition> transitions;
};

// `s32 count, pad[3]`, count circles, then count records; the loader's -1 ->
// 0xb applied.
EnvTransitions read_env_transitions(const GameplayFile& file);

// ---- Point lights ----

// Sections 0x7c and 0x78.
struct GameplayPointLight {
    std::array<float, 4> position{};  // x, y, z, and the squared radius
    u32 rgba = 0;
};

struct GameplayPointLights {
    std::vector<GameplayPointLight> lights;
    // The grid section's words as stored: 64 x 64 cells of 16 units after a
    // 0x10-byte header; a cell's word is the offset of its list `s32 count,
    // count x s32 light index`, 0 for none.
    std::vector<u32> grid;
};

GameplayPointLights read_point_lights(const GameplayFile& file);

}  // namespace openrac::assets
