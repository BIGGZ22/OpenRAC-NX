// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac): the mesh and storage
// layouts of crates/rc-engine/src/tfrag_render.rs, tie_render.rs,
// shrub_render.rs, shrub_billboard.rs, moby_render.rs, sky_render.rs,
// particle_render.rs, water_render.rs, fx_draw.rs and hud_render.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// What the world renderers draw a level from: the render-ready form of a
// level's terrain, ties, shrubs, mobys, sky, water and effect textures, built
// once when the level loads and uploaded by each renderer's load(). These are
// the inputs ReRAC's renderers feed their shaders, in OpenRAC's own types and
// without anything of Bevy. They depend on nothing in port/assets: the asset
// readers are bridged to them elsewhere.
//
// Conventions shared by every struct here:
//   - Positions are in game axes (Z up), game units (the formats' integer
//     units / 1024), unless a field says otherwise.
//   - Colours are the chip's display bytes as a little-endian word, R in the
//     low byte; 0x80 is 1.0 when a colour modulates a texture.
//   - Textures are TexturePool textures added with AlphaScale::Gs: RGBA8 of
//     the bytes the GS sees, texel alpha in the chip's units (0x80 opaque).
//   - Matrices are renderer/math.h's Mat4: column-major, game axes.
//   - Constants the game's code sets (draw-distance caps, near and far
//     culls) are not hard-coded: they come in WorldConstants, per game version
//     (profile.h).

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "renderer/math.h"
#include "renderer/texture.h"
#include "renderer/texture_pool.h"
#include "renderer/world/gs_pass.h"

namespace openrac::renderer::world {

// ---- Shared ----

// Values the game's level loader and renderers set that the world renderers
// replay. ReRAC read them from the RAC1 NTSC-U executable (NTSC-U
// addresses); the other games' are not known yet, and a bridge for them must
// supply its own.
struct WorldConstants {
    // TieProc's cap on every instance draw distance (written by the level
    // loader at NTSC-U 0x160f70: 720.0), game units.
    float tie_draw_cap = 0.0f;
    // ShrubProc's cap (NTSC-U gp-0x675c: 500.0) and the loader's minimum
    // instance draw distance (16.0), game units.
    float shrub_draw_cap = 0.0f;
    float shrub_draw_min = 0.0f;
    // MobyProc's cap on moby+0x32 (NTSC-U 0x15ff30: 500), game units.
    std::int32_t moby_draw_cap = 0;
    // TfragProc's far cull (NTSC-U 0x160ec0: 512000 integer units).
    float tfrag_far_cull = 0.0f;
    // PartProc's global far, 20.12 fixed point (500 units; 64 underwater).
    std::int32_t particle_far12 = 0;
    // The shrub billboard's fade band (F + 24 raises a billboard class's draw
    // distance) and width of the mesh / billboard cross-fade (8 units).
    float shrub_billboard_margin = 0.0f;
};

// RAC1 (NTSC-U as read by ReRAC; the PAL executable is assumed to set the
// same values, not checked).
inline constexpr WorldConstants kRac1Constants{720.0f, 500.0f, 16.0f, 500, 512000.0f, 500 << 12, 24.0f};

// An object's load-time occlusion word (tfrags, ties, mobys): the high byte
// selects a byte of the frame's 1,024-bit visibility mask, the low byte the
// bits within it. 0x7f80 (bit 1023, always set in a frame mask) means
// "always visible".
struct OcclusionBits {
    std::uint16_t word = 0x7f80;

    std::uint8_t byte() const { return static_cast<std::uint8_t>(word >> 8); }

    std::uint8_t bits() const { return static_cast<std::uint8_t>(word); }

    // The test TfragProc, TieProc and MobyProc make: mask[byte] & bits != 0.
    // An empty mask means no occlusion data: visible.
    bool visible(std::span<const std::uint8_t> mask) const {
        if (mask.empty()) {
            return true;
        }
        return byte() < mask.size() && (mask[byte()] & bits()) != 0;
    }
};

// A texture a renderer samples: a TexturePool texture, the game's own mip
// chain for it (levels 1 and up; the pool holds level 0) and the range of its
// texel alphas over every level, for the alpha-test split (GsPass draws()).
struct WorldTexture {
    TextureHandle texture = 0;
    std::vector<Rgba8Image> mip_levels;
    AlphaRange alpha;
};

// How a draw samples its texture (the ad-gif's CLAMP_1 and TEX1).
struct SamplerState {
    bool clamp_s = false;
    bool clamp_t = false;
    // TEX1 MMIN = LINEAR_MIPMAP_NEAREST: the shader picks the level with the
    // GS rule (bilinear inside it). Off: bilinear on level 0.
    bool mipmapped = false;
    bool nearest = false;  // TEX1 MMAG / MMIN NEAREST

    bool operator==(const SamplerState&) const = default;
};

// ---- Terrain (tfrag) ----
//
// Every triangle of all three strip lists (LOD 0, 1, 2) of every tfrag is in
// the batches, batched by texture and wrap state. Each frame TfragProc's
// decision per tfrag (tfrag_lod.h) picks which list a tfrag draws and which
// VU1 entry's morph and collapse passes run; the vertex shader replays them
// from the slots and vertex infos.

enum class TfragTier : std::uint8_t {
    None,   // a common slot or entry
    Lod01,  // written by a LOD-01 primary: morphs between D1 and D0
    Lod0,   // written by a LOD-0 primary: morphs between D2 and D1
};

// One VU position slot (tfrag, position).
struct TfragSlot {
    std::array<float, 3> position{};  // world, game units
    std::uint32_t rgba = 0x80808080;  // the LightTfrags (baked) colour
    // For a slot a morphing primary entry stores into: its tier and its two
    // parent slots (global slot indices).
    TfragTier tier = TfragTier::None;
    std::uint32_t parent1 = 0;
    std::uint32_t parent2 = 0;
    // The LightTfrags record's normal for the point lights: azimuth | elevation
    // << 8 (256 steps a turn); has_normal false when the slot has no record.
    std::uint8_t azimuth = 0;
    std::uint8_t elevation = 0;
    bool has_normal = false;
};

// One vertex-info entry (tfrag, entry).
struct TfragVertexInfo {
    float s = 0.0f;  // GS S and T (Q = 1): normalised texture coordinates
    float t = 0.0f;
    std::uint32_t slot = 0;  // global slot index
    // LOD-01 / LOD-0 entries (primary or extra) collapse onto their parent-1
    // entry (a global vertex-info index) when both parents are beyond the
    // tier's collapse distance; parent 2 is a slot.
    TfragTier tier = TfragTier::None;
    std::uint32_t parent1_info = 0;
    std::uint32_t parent2_slot = 0;
};

struct TfragVertex {
    std::uint32_t info = 0;   // global vertex-info index
    std::uint16_t tfrag = 0;  // tfrag index
    std::uint8_t list = 0;    // strip list: 0, 1 or 2
    std::int16_t lod_k = 0;   // the ad-gif's TEX1 K, s12 in 1/16 mip levels
};

struct TfragBatch {
    std::uint32_t texture = 0;  // index into TfragDrawData::textures
    SamplerState sampler{false, false, true, false};
    AlphaRange vertex_alpha;  // range of the slot alphas (Af) drawn here
    std::vector<TfragVertex> vertices;
    std::vector<std::uint32_t> indices;  // triangle list
};

// What TfragProc reads of one tfrag.
struct TfragCull {
    std::array<float, 4> sphere{};                    // centre and radius, integer units
    std::array<std::array<float, 3>, 8> corners{};    // the clip box, integer units
    bool base_only = false;                           // header flag: always LOD 2
    OcclusionBits occlusion;
};

struct TfragDrawData {
    // The tfrag block header's LOD base distance L: D0, D1, D2 = 6L, 4L, 2L
    // (game units).
    float lod_base = 0.0f;
    std::vector<TfragCull> tfrags;
    std::vector<TfragSlot> slots;
    std::vector<TfragVertexInfo> infos;
    std::vector<TfragBatch> batches;
    std::vector<WorldTexture> textures;
    WorldConstants constants;
};

// ---- Ties ----

struct TieVertex {
    std::array<float, 3> position{};  // class space, game units (position x scale / 1024)
    std::array<float, 3> delta{};     // morph delta, same units (fat vertices: position + k * delta)
    float s = 0.0f;
    float t = 0.0f;
    std::uint8_t slot = 0;  // light slot 0..63 of the instance's colour table
    // A fat vertex blends two more slots' colours by the instance's morph
    // weights (vu::fat_colour_lane).
    bool fat = false;
    std::uint8_t morph_slot1 = 0;
    std::uint8_t morph_slot2 = 0;
    std::uint8_t lod = 0;    // 0, 1, 2
    std::int16_t lod_k = 0;  // TEX1 K, s12 in 1/16
};

struct TiePart {
    std::uint32_t texture = 0;  // index into TieDrawData::textures
    SamplerState sampler{false, false, true, false};
    std::vector<TieVertex> vertices;
    std::vector<std::uint32_t> indices;
    std::uint64_t slots_used = 0;  // light slots any vertex reads
};

struct TieClass {
    std::array<float, 3> lod_distances{};  // the class's near, mid and far distances
    std::vector<TiePart> parts;
};

struct TieInstance {
    std::uint32_t tie_class = 0;
    Mat4 matrix = identity();          // class to world (may scale, shear, mirror)
    std::array<float, 4> sphere{};     // world centre and radius (LightTies' instance centre)
    float draw_distance = 0.0f;        // the instance's s32 distance as float; 0 never drawn
    std::array<std::uint32_t, 64> colours{};  // LightTies' result per slot
    // The class normals LightTies lights with (s16, / 32768), per slot, for
    // the point lights.
    std::array<std::array<std::int16_t, 3>, 64> normals{};
    OcclusionBits occlusion;
};

struct TieDrawData {
    std::vector<TieClass> classes;
    std::vector<TieInstance> instances;
    std::vector<WorldTexture> textures;
    WorldConstants constants;
};

// ---- Shrubs ----

struct ShrubVertex {
    std::array<float, 3> position{};  // class space, game units
    float s = 0.0f;
    float t = 0.0f;
    std::uint8_t palette_entry = 0;  // the instance palette entry VU1 reads
    std::int16_t lod_k = 0;
};

struct ShrubPart {
    std::uint32_t texture = 0;
    SamplerState sampler{false, false, true, false};
    std::vector<ShrubVertex> vertices;
    std::vector<std::uint32_t> indices;
};

struct ShrubClass {
    std::vector<ShrubPart> parts;
    bool skipped = false;           // class mode bit 0: never drawn
    std::uint8_t sway_mode = 0;     // (mode bits & 6) >> 1: 0 no wind sway
    // The billboard (far LOD) when the class has one: its fade distance F
    // (trunc(fade_distance) & 0xff; 0 = billboard only), texture, MXL and K.
    bool has_billboard = false;
    float billboard_fade = 0.0f;
    std::uint32_t billboard_texture = 0;
    std::uint8_t billboard_max_level = 0;
    float billboard_k = 0.0f;  // mip levels
};

struct ShrubInstance {
    std::uint32_t shrub_class = 0;
    Mat4 matrix = identity();
    std::array<float, 4> sphere{};  // world centre and radius (LightShrubs' centre)
    float draw_distance = 0.0f;     // the instance's f32, before the loader's rules
    std::array<std::uint32_t, 24> palette{};  // LightShrubs' result
    std::array<std::array<std::int16_t, 3>, 24> normals{};
    // The billboard's world extent (width, height, z offset), from the
    // class's billboard record and the instance's column lengths.
    std::array<float, 3> billboard_extent{};
};

struct ShrubDrawData {
    std::vector<ShrubClass> classes;
    std::vector<ShrubInstance> instances;
    std::vector<WorldTexture> textures;
    WorldConstants constants;
};

// ---- Mobys ----

inline constexpr std::uint32_t kMobyGreyTexture = 0xffffffffu;  // texture -1: the 0x80 grey texel

struct MobyVertex {
    std::array<std::int16_t, 3> position{};  // packed model units (skinned before scaling)
    float s = 0.0f;
    float t = 0.0f;
    std::uint8_t azimuth = 0;  // normal, into the normal table
    std::uint8_t elevation = 0;
    std::uint8_t joint_count = 1;  // 0: the identity (a low LOD drawn with no joints)
    std::array<std::uint8_t, 3> joints{};
    std::array<std::uint16_t, 3> weights{};  // / 256
    std::uint32_t multiplier = 0x80808080;   // the RGBA multiplier the EE packs the light with
    bool glow = false;                       // in a glow packet
    std::uint32_t vertex_id = 0;
};

struct MobyPart {
    std::uint32_t texture = kMobyGreyTexture;  // index into MobyDrawData::textures
    bool glow = false;
    AlphaRange multiplier_alpha;
    std::vector<MobyVertex> vertices;
    std::vector<std::uint32_t> indices;
};

enum class MobyMetalKind : std::uint8_t {
    Chrome,  // texture -2: the level's 128 x 128 chrome map
    Glass,   // texture -3: the 64 x 64 glass map
};

struct MobyMetalPart {
    MobyMetalKind kind = MobyMetalKind::Chrome;
    std::vector<MobyVertex> vertices;  // s, t unused: sphere-mapped
    std::vector<std::uint32_t> indices;
};

struct MobyClass {
    std::vector<MobyPart> high;
    std::vector<MobyPart> low;  // empty: no low LOD
    std::vector<MobyMetalPart> metal;
    std::uint8_t lod_trans = 0xff;     // class byte 0xe: the LOD switch, in units of 1024
    std::array<float, 4> sphere{};     // class header sphere, packed units
    bool caster = false;               // has a shadow block: deferred after the shadow pass
    std::uint32_t glow_rgba = 0;       // class glow colour (0: none)
};

// The light block MobyProc builds per moby (ReRAC's moby_light, f32): three
// directional lights in model space, their colours, back-light factors and
// the ambient colour.
struct MobyLights {
    std::array<std::array<float, 4>, 3> rows{};    // row j = (L0[j], L1[j], L2[j], 0)
    std::array<std::array<float, 4>, 3> colours{};  // C0..C2 (1.0 adds 128)
    std::array<float, 4> neg_k{0, 0, 0, 1};         // (-|K0|, -|K1|, -|K2|, 1)
    std::array<float, 4> ambient{128, 128, 128, 128};  // colour counts 0..255, alpha
};

enum class MobyColourMode : std::uint8_t {
    GpuLight,  // light in the vertex shader
    Unlit,     // every vertex 0x80 (no light data)
};

struct MobyInstance {
    std::uint32_t moby_class = 0;
    std::array<float, 3> position{};
    std::array<std::array<float, 3>, 3> rows{{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};  // moby+0xc0
    float scale = 1.0f;                 // class scale x instance scale
    std::int32_t draw_distance = 0;     // moby+0x32
    MobyLights lights;
    MobyColourMode colour_mode = MobyColourMode::GpuLight;
    OcclusionBits occlusion;
    std::uint32_t palette_base = 0;     // first joint matrix of this instance
    std::uint32_t palette_slots = 1;
};

struct MobyDrawData {
    std::vector<MobyClass> classes;
    std::vector<MobyInstance> instances;
    std::vector<WorldTexture> textures;
    WorldTexture chrome;
    WorldTexture glass;
    // The (cos, sin) normal-decode table the game's lighting reads, 256
    // entries; from the player's executable, never embedded.
    std::vector<std::array<float, 2>> normal_table;
    WorldConstants constants;
};

// ---- Sky ----

struct SkyVertex {
    std::array<float, 3> position{};  // the shell's raw s16 integers, as floats
    float s = 0.0f;
    float t = 0.0f;
    std::uint32_t rgba = 0x80808080;  // the GS vertex colour
};

// One sky draw: a run of consecutive faces of one shell with one texture,
// in the game's draw order.
struct SkyDraw {
    std::uint32_t shell = 0;
    bool textured = false;
    std::uint32_t texture = 0;  // index into SkyDrawData::textures (sky texture index)
    std::vector<SkyVertex> vertices;  // triangle list
};

struct SkyDrawData {
    std::vector<SkyDraw> draws;
    // The sky's textures by sky texture index (the stars use them too).
    std::vector<WorldTexture> textures;
    // The star step's place: the stars are drawn before draws[stars_before]
    // (draws.size(): after the last).
    std::optional<std::size_t> stars_before;
};

// ---- Particles ----

struct ParticleDrawData {
    // The part textures, one layer each (all the same size; 32 x 32 in RAC1),
    // RGBA with the chip's alpha. The renderer keeps them in one texture array
    // (one draw per blend group in sorted order needs every layer at once), so
    // they are images here, not pool textures.
    std::vector<Rgba8Image> layers;
};

// ---- Water ----

struct WaterStripVertex {
    std::array<float, 4> position{};  // game x, y, z0, z1
    float s = 0.0f;
    float t = 0.0f;
    std::uint8_t wobble_group = 0;  // hash & 7: which wobble offset the vertex takes
};

struct WaterStripLayer {
    std::uint32_t texture = 0;  // index into WaterDrawData::textures (the FX textures)
    std::uint8_t fix = 0x80;    // ALPHA FIX
};

struct WaterStrip {
    std::vector<WaterStripVertex> vertices;  // one GS triangle strip
    std::uint32_t rgba = 0;
    std::array<WaterStripLayer, 2> layers{};
};

struct WaterPatch {
    WaterStripLayer water;  // the water pass (animated UV)
    WaterStripLayer env;    // the env pass (sphere-map UV)
};

struct WaterDrawData {
    std::vector<WaterStrip> strips;  // in callback order, layer 1 then 2 each
    // The ripple patches: the grid offsets (into the 17 x 17 grid) of the 46
    // vertices of a sub-block's strip, and the patches.
    std::vector<std::uint16_t> patch_strip_order;
    std::vector<WaterPatch> patches;
    std::vector<WorldTexture> textures;
};

// ---- Effect textures (draw callbacks), the HUD ----

struct EffectsDrawData {
    std::vector<WorldTexture> textures;  // the FX textures by index
};

struct HudDrawData {
    // Every HUD frame and FX texture the 2D pass samples, by the indices
    // HudPrim::texture names; packed into one atlas at load. RGBA with the
    // chip's alpha.
    std::vector<Rgba8Image> textures;
};

}  // namespace openrac::renderer::world
