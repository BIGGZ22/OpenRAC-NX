// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The renderer's shape, after OpenGOAL's (OPENGOAL_NOTES.md, section 3): a
// frame is a list of per-subsystem renderers run in the order the game draws
// (RENDERER.md, section 1: sky, terrain, ties, shrubs, mobys, particles, then
// the 2D), each one drawing its own kind of thing on the GPU from what the
// game prepared for it. Nothing models the console's graphics hardware: a
// renderer either reads the packets the game built for its subsystem (the
// direct renderer, for the 2D path) or draws converted assets with the
// camera, visibility and lights the game computed (the world renderers, see
// subsystems.h).
//
// How the game hands a frame over is still open (DESIGN.md, section 4, item
// 3). FrameInput carries, per bucket, the bytes the game built for it at its
// sync point (VU1_sendChain) and the camera; it grows as the hand-off is
// decided.

#pragma once

#include <array>
#include <functional>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "renderer/math.h"
#include "renderer/texture_pool.h"

namespace openrac::renderer {

// What the game draws, in DrawWorld's order.
enum class Bucket : std::uint8_t {
    Sky,
    Terrain,
    Ties,
    Shrubs,
    Mobys,
    Particles,
    Hud,  // the 2D path: HUD, text, fades, letterbox (GIF packets)
    Count,
};

inline constexpr std::size_t kBucketCount = static_cast<std::size_t>(Bucket::Count);

const char* bucket_name(Bucket bucket);

struct Camera {
    Mat4 view = identity();        // world (game axes) to view
    Mat4 projection = identity();  // view to clip, reversed depth (math.h)
    Vec3 position{};
};

// A textured quad the game drew in the world from a draw callback (rac1's func_001F7EF8, ReRAC's
// FastDrawQuadReal): four corners in world space, drawn as the strip 0 1 2, 1 2 3, with the GS
// registers the game gave it. Colours are the chip's RGBA bytes (0x80 = 1.0); ST are normalised.
struct EffectQuad {
    float corner[4][4];
    std::uint32_t rgba[4];
    float st[4][2];
    std::uint64_t clamp, tex0, tex1, alpha;
    // FrameInput::effect_uploads[uploads], [uploads + 1]: the CLUT and the pixels put at TEX0's
    // blocks just before this quad (the game pages its effect textures through the same blocks
    // during a frame); -1 when the texture is already in the pool.
    int uploads = -1;
    // When nonzero, the texture's and CLUT's blocks in the pool instead of TEX0's (a texture the
    // window keeps at a key of its own, past the GS's 14-bit blocks).
    std::uint32_t tbp = 0, cbp = 0;
};

// A strip the game sent to its VU1 strip program (rac1: func_L00_001FDE48's packets: the seas,
// liquids and other callback meshes): count vertices from first in FrameInput::strip_vertices, in
// world space, drawn as a triangle strip with the GS registers in effect when the game sent it.
struct EffectVertex {
    float x, y, z;
    float s, t;
    std::uint32_t rgba;  // the chip's bytes, R low (0x80 = 1.0)
};

struct EffectStrip {
    std::uint32_t first = 0, count = 0;
    std::uint64_t clamp = 0, tex0 = 0, tex1 = 0, alpha = 0x44;
    // FrameInput::effect_uploads[uploads..uploads + upload_count): the images the game sent to
    // TEX0's blocks just before the strip; put in the pool before it is drawn.
    int uploads = -1, upload_count = 0;
};

// A textured quad of the 2D layer the game drew through one of its 2D routines (rac1:
// DrawTexturedQuad, the HUD's sprite routines), drawn natively where its marker comes in the 2D
// path (GifInterpreter): four corners in strip order (0-1-2, 1-2-3) in the chip's 12.4 screen
// coordinates as the game computed them, with their texel coordinates (12.4 both ways), the depth,
// the colour (RGBAQ's low word) and the texture (TEX0).
struct UiQuad {
    std::uint32_t x[4] = {}, y[4] = {};
    std::uint32_t u[4] = {}, v[4] = {};
    std::uint32_t z = 0xFFFFF0;
    std::uint32_t rgba = 0;
    std::uint64_t tex0 = 0;
    std::uint32_t flags = 0;  // kUiGlyph: a glyph of text (drawn from a distance field); kUiSprite
};

// UiQuad::flags: kUiSprite, an axis-aligned rectangle the game sends as a sprite (corners 0 and 3),
// whose texel mapping follows the sprite's rules (a panel drawn off-screen and sampled back).
inline constexpr std::uint32_t kUiSprite = 2;

inline constexpr std::uint32_t kUiGlyph = 1;

struct FrameInput {
    // The packets the game built for each bucket this frame (empty when a
    // bucket has nothing, or its renderer draws from elsewhere).
    std::array<std::span<const std::uint8_t>, kBucketCount> packets{};
    // The 2D path's data sent before the frame's first VU1 program, which the world comes after:
    // the frame's clear, and what the page menus put under their frame objects (the snapshot of
    // the frame they opened over, darkened). Drawn before the world (DirectBackground).
    std::span<const std::uint8_t> direct_before_world{};
    Camera camera;
    // The frame's world effect quads, in the order the game drew them.
    std::span<const EffectQuad> effects{};
    std::span<const ImageUpload> effect_uploads{};
    std::span<const EffectVertex> strip_vertices{};
    std::span<const EffectStrip> strips{};
    // The 2D layer's quads drawn natively (UiQuad), indexed by their markers in the 2D path.
    std::span<const UiQuad> ui_quads{};
    std::array<float, 4> clear_colour{0.0f, 0.0f, 0.0f, 1.0f};
    // The target already holds the picture to start from (a tick that draws no world of its own):
    // only depth is cleared.
    bool keep_colour = false;
    // Called with the target as drawn before the HUD's renderer (the world, without the 2D over
    // it): what a copy of the frame the game reads back should hold.
    std::function<void()> before_hud;
    std::uint64_t frame = 0;
    double seconds = 0.0;
};

// Where a frame is drawn: a framebuffer (0 is the window) and its size.
struct RenderTarget {
    unsigned framebuffer = 0;
    int width = 0;
    int height = 0;
};

struct FrameStats {
    int draw_calls = 0;
    std::uint64_t triangles = 0;
};

// What every renderer shares during a frame.
struct RenderState {
    TexturePool& textures;
    RenderTarget target;
    FrameStats stats;
};

// One subsystem's renderer (OpenGOAL's BucketRenderer).
class BucketRenderer {
public:
    BucketRenderer(std::string name, Bucket bucket) : m_name(std::move(name)), m_bucket(bucket) {}

    virtual ~BucketRenderer() = default;
    BucketRenderer(const BucketRenderer&) = delete;
    BucketRenderer& operator=(const BucketRenderer&) = delete;

    // Create GL objects (shaders, buffers); the context is current.
    virtual bool init(RenderState& state, std::string& error) {
        (void)state;
        (void)error;
        return true;
    }

    // Draw this subsystem's part of the frame.
    virtual void render(const FrameInput& input, RenderState& state) = 0;

    // Free GL objects; the context is still current.
    virtual void release() {}

    const std::string& name() const { return m_name; }

    Bucket bucket() const { return m_bucket; }

    bool enabled = true;

private:
    std::string m_name;
    Bucket m_bucket;
};

class Renderer {
public:
    Renderer() = default;
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    // After gl::load(): initialise every renderer added so far. Renderers
    // added later are initialised as they are added.
    bool init(std::string& error);

    // Appended in drawing order. Returns the renderer, or null if its init failed.
    BucketRenderer* add(std::unique_ptr<BucketRenderer> renderer, std::string& error);

    // The game's subsystems in DrawWorld's order: the placeholders of
    // subsystems.h for the world, and a DirectRenderer for the 2D.
    bool add_game_renderers(std::string& error);

    // Draw one frame into `target`: clear it (depth to 0, for reversed
    // depth), then run each enabled renderer in order.
    void render(const FrameInput& input, const RenderTarget& target);

    // Free every GL object (the context must still be current).
    void release();

    // Drop every subsystem renderer (released first) but keep the texture pool: the console's GS
    // memory outlives a change of level, and what the game uploaded stays there.
    void clear_renderers();

    TexturePool& textures() { return m_textures; }

    std::span<const std::unique_ptr<BucketRenderer>> renderers() const { return m_renderers; }

    const FrameStats& last_stats() const { return m_stats; }

private:
    TexturePool m_textures;
    std::vector<std::unique_ptr<BucketRenderer>> m_renderers;
    FrameStats m_stats;
    bool m_initialised = false;
};

}  // namespace openrac::renderer
