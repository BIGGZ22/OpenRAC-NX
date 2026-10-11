// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac): the per-frame state its
// renderers read from the game (crates/rc-engine/src/game_camera.rs,
// world_lights.rs, moby_render.rs, moby_anim.rs, sky_render.rs, sky_stars.rs,
// particle_render.rs, shadow_render.rs, water_render.rs, fx_draw.rs,
// fog_state.rs, screen_tint.rs, mirror_render.rs, hud_render.rs):
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// What the game computes each frame for the world renderers: the fog, the
// occlusion mask, the point lights, the mobys' run-time state and joint
// matrices, the particles, the shadow volumes, the sky's star sprites and
// shell rotations, the water's animation, the draw callbacks' primitives,
// the full-screen effects and the 2D primitives. The renderers read it
// through WorldContext::frame(); the camera comes from FrameInput.
//
// Spans point into the caller's memory, which must outlive the frame's
// Renderer::render. An empty span means "nothing this frame" (or, for the
// occlusion mask and the moby states, "the load-time values").

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "renderer/math.h"
#include "renderer/renderer.h"
#include "renderer/world/draw_data.h"
#include "renderer/world/fog.h"

namespace openrac::renderer::world {

// The camera as the game's per-frame code uses it: the eye and the rotation
// rows (forward, left, up) in game axes, and the half-angle tangents of the
// view (UpdateViewContext's frustum planes).
struct WorldCamera {
    Vec3 eye{};
    Vec3 forward{0, 1, 0};
    Vec3 left{-1, 0, 0};
    Vec3 up{0, 0, 1};
    float tan_x = 0.63f;
    float tan_y = 0.63f * 0.775f;

    // The VU's camera rows (camera space x right, y down, z forward).
    std::array<Vec3, 3> vu_rows() const;
    // A point (game units) in that camera space, game units.
    Vec3 to_camera(const Vec3& p) const;

    // From a renderer Camera whose view is world (game axes) to GL view
    // space (renderer/math.h): the rows of the view matrix, the tangents from
    // the projection's diagonal.
    static WorldCamera from(const Camera& camera);
};

// The camera matrices of the game's own projection, as ReRAC's GameProjection
// builds them for the console's numbers (InitViewContext, ReRAC's reading):
// near 32 and far 745472 integer units, and the GS's 24-bit Z, larger nearer,
// mapped to GL's window depth as Z / 2^24 (reversed depth, cleared to 0,
// tested GEQUAL). tan_y is tan_x x the profile's frame_y_ratio (profile.h).
Mat4 game_projection(float tan_x, float tan_y);

// The game's camera: `eye` in game units, rows forward / left / up.
Camera game_camera(const Vec3& eye, const Vec3& forward, const Vec3& up, float tan_x, float tan_y);

// One slot of the point-light bank (UpdateAllPointLights): position (game
// units), radius, colour (1.0 adds 128 to a colour byte) and intensity (the
// back-light factor w: d -> max(d, d * w)). Free slots have radius 0.
struct PointLight {
    std::array<float, 3> position{};
    float radius = 0.0f;
    std::array<float, 3> colour{};
    float intensity = 0.0f;
};

inline constexpr std::size_t kPointLightSlots = 8;
// A point-light nibble list: up to four bank slots, low nibble first, 0xf
// ends it; 0xffff names none.
inline constexpr std::uint16_t kNoLights = 0xffff;

// What MobyProc reads of a moby at run time besides its placement. Absent
// (an empty WorldFrame::mobys), every instance uses its load-time values.
struct MobyState {
    bool hidden = false;          // deleted, mode bit 1, or not drawn by the moby loop
    std::uint8_t alpha = 0x80;    // moby+0x23
    std::uint16_t mode = 0;       // moby+0x34: 0x10 glow list, 0x200 additive, 8 fading
    std::uint32_t glow = 0;       // moby+0x90 (RGB)
    std::uint8_t shine_distance = 0;  // moby+0x73 (0: no metal pass)
    std::optional<std::int16_t> draw_distance;  // moby+0x32 when the game rewrote it
    std::optional<std::array<float, 4>> sequence_sphere;  // packed units; else the class sphere
    bool late = true;  // a caster's draws after the shadow pass (MobyProc's deferral)
    // A moving moby's placement (else the instance's).
    std::optional<std::array<float, 3>> position;
    std::optional<std::array<std::array<float, 3>, 3>> rows;
    std::optional<float> scale;
    std::array<std::int16_t, 2> uv_scroll{};  // a class texture scroll, 1/4096
};

// One joint matrix of the palette: rows 0..2 are the images of the model axes,
// row 3 the translation (MobyAnimEval's layout), packed model units.
struct JointMatrix {
    std::array<std::array<float, 4>, 4> rows{{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
};

// A sky star sprite record (the star step's generators write them; SkySpriteProc
// draws them): a direction on a radius-50 shell around the eye.
struct SkyStar {
    std::array<float, 3> position{};
    float size = 0.0f;
    float sin = 0.0f;  // the VU0 sine and cosine of the record's rotation
    float cos = 1.0f;
    std::uint32_t rgba = 0;
    std::int8_t texture = -1;  // sky texture index; < 0 skipped
    bool additive = true;      // ALPHA_1 0x48 (every retail star)
};

// A live particle record, decoded from the particle pool (PartProc's inputs).
struct ParticleRecord {
    std::uint8_t kind = 0;  // 0 sprite, 1 flat quad, 2 line, 3 ribbon
    std::uint8_t layer = 0;
    bool additive = false;     // ALPHA 0x48 (else 0x44)
    std::uint8_t alpha = 0x80;  // record byte 7
    std::uint8_t rotation = 0;  // record byte 8: 256 steps a turn
    std::uint8_t near_far = 0;  // record byte 9: near nibble low, far nibble high
    std::array<float, 3> position{};
    std::uint32_t rgba = 0x80808080;
    float size = 0.0f;  // in 1/420000 units
    // Lines and ribbons: the second end, its colour, the width and its taper.
    std::array<float, 3> end{};
    std::uint32_t rgba_end = 0x80808080;
    float width = 0.0f;
    float width_taper = 1.0f;
};

// One callback slot's water animation: the z blend, this layer's scroll and
// the eight wobble offsets (VU0 sine on the CPU).
struct WaterStripFrame {
    float z_blend = 0.0f;
    std::array<float, 2> scroll{};
    std::array<std::array<float, 2>, 8> wobble{};
};

inline constexpr std::size_t kRippleGrid = 17;
inline constexpr std::size_t kRippleStrip = 46;

// One ripple patch's frame: which of its 16 sub-blocks draw, its 17 x 17
// grid (position, sphere-map UV, grey), the 46 animated water UVs.
struct RipplePatchFrame {
    bool visible = false;
    std::uint16_t mask = 0;
    std::array<std::array<float, 5>, kRippleGrid * kRippleGrid> grid{};  // x, y, z, env u, env v
    std::array<std::uint32_t, kRippleGrid * kRippleGrid> rgba{};
    std::array<std::array<float, 2>, kRippleStrip> water_uv{};
    std::optional<std::uint8_t> fix_water;  // a FIX set at run time
    std::optional<std::uint8_t> fix_env;
};

// A draw callback's primitives (FastDrawQuadReal quads, DrawEnvOverlayMesh
// strips) as triangles, with one texture and one equation.
struct EffectVertex {
    std::array<float, 3> position{};  // game units
    float s = 0.0f;
    float t = 0.0f;
    std::uint32_t rgba = 0x80808080;
};

enum class EffectBlend : std::uint8_t {
    Mix,       // ALPHA 0x44
    Add,       // ALPHA 0x48
    Subtract,  // ALPHA 0x62 with As = FIX: Cd - Cs * FIX
    Opaque,    // FIX 0x80 with Z: an opaque world surface
};

struct EffectGroup {
    std::uint32_t texture = 0;  // index into EffectsDrawData::textures
    EffectBlend blend = EffectBlend::Mix;
    bool fog = true;
    std::vector<EffectVertex> vertices;
    std::vector<std::uint32_t> indices;
};

// A full-screen tint the game draws with an untextured sprite: Cv = ((A -
// B) * As >> 7) + D on the frame buffer's bytes, A / B / D from the low byte of
// its ALPHA_1 (bits 0-1 A, 2-3 B, 6-7 D: 0 Cs, 1 Cd, 2 zero).
struct ScreenTint {
    std::uint32_t rgba = 0;
    std::uint8_t alpha_register = 0x44;
};

// The Visibomb's missile view overlay (ALPHA_1 0x42 rectangles): the frame's
// rectangle in target pixels, the band alpha and rows.
struct MissileView {
    std::array<float, 4> viewport{};  // x, y, width, height
    std::uint8_t band_alpha = 0x17;
    std::uint32_t frame_rows = 448;  // the profile's frame_height
    std::uint32_t band_period = 34;
    std::uint32_t band_rows = 31;
};

struct PostFrame {
    // In the game's order: the world's tints, the underwater tint, the fade.
    std::vector<ScreenTint> tints;  // at most four
    std::optional<std::uint32_t> underwater;  // RGBA
    std::optional<MissileView> missile_view;
    bool mirror = false;
    std::optional<std::uint32_t> fade;  // the full-screen black quad's RGBA
};

// A 2D primitive of the HUD pass: four corners in strip order (triangles 0 1
// 2, 1 3 2), in 1/16 game pixels of the profile's frame, texel UVs.
struct HudPrim {
    std::optional<std::uint32_t> texture;  // index into HudDrawData::textures
    std::array<std::array<std::int32_t, 2>, 4> position{};
    std::array<std::array<float, 2>, 4> uv{};
    std::uint32_t rgba = 0x80808080;
    std::array<std::int32_t, 4> scissor{0, 511, 0, 415};  // x0, x1, y0, y1 inclusive
    bool repeat = false;   // CLAMP_1 = 0: texel coordinates wrap
    bool nearest = false;  // TEX1 point sampling
    bool additive = false;  // ALPHA_1 0x48
};

struct WorldFrame {
    WorldFog fog;
    std::uint64_t tick = 0;  // the 60 Hz frame counter
    // The occlusion mask (128 bytes), empty: everything visible.
    std::span<const std::uint8_t> occlusion;

    // Point lights and the nibble lists naming them per tfrag, tie instance and
    // shrub instance (empty lists: none).
    std::array<PointLight, kPointLightSlots> lights{};
    std::span<const std::uint16_t> tfrag_lights;
    std::span<const std::uint16_t> tie_lights;
    std::span<const std::uint16_t> shrub_lights;

    std::span<const MobyState> mobys;
    std::span<const JointMatrix> joints;

    // The sky: each shell's SkyM (empty: identity) and the star sprites.
    std::span<const Mat4> sky_shells;
    std::span<const SkyStar> sky_stars;
    bool clear_before_sky = true;

    std::span<const ParticleRecord> particles;
    std::int32_t particle_far12 = 0;  // 0: the constants' value

    // Shadow volumes: closed prisms as triangles (three points each).
    std::span<const std::array<float, 3>> shadow_volumes;

    std::span<const WaterStripFrame> water_strips;  // per strip layer slot
    std::span<const RipplePatchFrame> ripples;

    // Draw callbacks: list 1 after the mobys, list 2 after the particles.
    std::span<const EffectGroup> effects_list1;
    std::span<const EffectGroup> effects_list2;

    PostFrame post;
    std::span<const HudPrim> hud;

    // Debug: tint LOD levels (tfrag LOD 1 red, LOD 2 blue, clip path green).
    bool lod_tint = false;
};

}  // namespace openrac::renderer::world
