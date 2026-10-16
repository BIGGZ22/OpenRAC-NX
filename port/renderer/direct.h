// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The direct renderer: the GIF packets the game builds for its 2D path (HUD,
// text, fades, letterbox, the screen clears) turned into GPU draw calls, after
// OpenGOAL's DirectRenderer (OPENGOAL_NOTES.md, sections 4.4 and 5.1).
//
// Two halves:
//
//   - GifInterpreter, on the CPU: reads GIF tags (PACKED, REGLIST, IMAGE)
//     and A+D register writes, keeps the drawing registers' values, assembles
//     each primitive's vertices into triangles, and groups them into draws
//     that share one GPU state. Image transfers go to the texture pool. It
//     makes no GL calls, so it is tested on its own.
//   - DirectRenderer, the bucket renderer: runs the interpreter over its
//     bucket's packets and issues one draw call per group, with the state
//     translated to GL (blend equation, depth test, alpha test in the shader,
//     scissor, texture).
//
// This is a translation of register state to draw calls. It rasterises
// nothing itself and keeps no image of the chip's memory: textures come from
// the pool, frame buffer and depth buffer are the GPU's.
//
// Conventions, from the chip and OpenGOAL's shaders: colours are in the
// chip's units (0x80 is 1.0 when modulating), the alpha test compares the
// chip's alpha, depth is reversed (bigger is nearer; cleared to 0), and a
// failed alpha test that still writes colour or depth is drawn twice
// (the "double draw").

#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "renderer/gs.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"
#include "renderer/texture_pool.h"

namespace openrac::renderer {

// The chip blends Cv = (A - B) * C + D, A, B, D each one of Cs, Cd, 0 and C
// one of As, Ad, FIX (0x80 = 1.0). Expanded, the colour is a multiple of Cs
// plus a multiple of Cd, each multiple k * C + m with k in {-1, 0, 1} and m
// in {0, 1}; GL's factors and equation express every such pair whose sign
// works out, which covers every mode the games use.
struct GlBlend {
    unsigned src = 0x0001;       // GL_ONE
    unsigned dst = 0;            // GL_ZERO
    unsigned equation = 0x8006;  // GL_FUNC_ADD
    float constant = 0.0f;
    bool exact = true;
};
GlBlend translate_blend(const gs::Alpha& alpha);


struct DirectConfig {
    // The game's frame buffer: 512 x 448 for PAL Ratchet & Clank.
    int screen_width = 512;
    int screen_height = 448;
    // The game's display buffer (a block address; its draw buffer is g_draw_buffer_block). Any
    // other target is drawn off screen, to be sampled later, however wide it is (the save page
    // draws its 512 x 128 hint panel at 0x1E00, right after the draw buffer).
    std::uint32_t display_block = 0x0000;
};

// One vertex as the GPU gets it (32 bytes).
struct DirectVertex {
    float x, y, z;         // normalised device coordinates; z from the chip's depth
    float s, t, q;         // texture coordinates times q (divided per pixel)
    std::uint8_t rgba[4];  // the chip's colour, 0x80 = 1.0
    float fog;             // 1: no fog, 0: all fog colour
};

static_assert(sizeof(DirectVertex) == 32);

// The GPU state of a draw: everything a register write can change that the
// draw call depends on. Two draws with equal states are merged.
struct DirectState {
    bool textured = false;
    TextureHandle texture = 0;
    bool texture_full_alpha = false;  // the texture's alpha runs to 0xFF, not 0x80
    bool tcc = false;
    gs::Tfx tfx = gs::Tfx::Modulate;
    bool linear_mag = false;
    bool linear_min = false;
    bool clamp_s = false;
    bool clamp_t = false;
    bool blend = false;
    gs::Alpha alpha{0, 1, 0, 1, 0};
    gs::Test test{};
    bool depth_write = true;
    std::uint32_t fbmsk = 0;
    gs::Scissor scissor{0, 511, 0, 447};
    bool fog = false;
    std::uint32_t fog_colour = 0;
    // Textured from a frame buffer, or from a copy of part of one (the game's blur, bands and
    // washes): the texture is the render target as drawn so far.
    bool frame_source = false;
    // Textured from the frame snapshot (FrameSnapshot): pixels the game read back from its frame
    // buffer and sends back into it.
    bool snapshot = false;
    // Drawn into an off-screen target (the GS block of a FRAME narrower than the screen: the menu's
    // panels), or textured from one; 0 for the main frame / a texture of the pool.
    std::uint32_t target = 0;
    std::uint32_t source = 0;
    // A glyph of the 2D layer's text drawn natively: the texture is the font's distance field
    // (TexturePool::distance_field), its alpha the distance to the glyphs' edge.
    bool distance_field = false;
    TextureHandle field = 0;  // its distance field, beside the texture itself

    bool operator==(const DirectState&) const = default;
};

struct DirectDraw {
    DirectState state;
    std::uint32_t first = 0;  // first vertex
    std::uint32_t count = 0;  // vertices (three per triangle)
    // Not 0: no vertices, the mobys a moby marker names, drawn into state.target here
    // (FrameInput::target_mobys), about the target's centre (its XYOFFSET's 2048), in its pixels.
    std::uint64_t mobys = 0;
    float centre_x = 0.0f, centre_y = 0.0f;
};

// Debugging: while set, every sprite and primitive the 2D path draws is logged (OPENRAC_DUMP_DRAWS).
inline bool g_dump_draws = false;

// An off-screen target covers this many chip pixels across and down (the largest the games' panels
// use), drawn at kOffscreenScale times that resolution.
inline constexpr int kOffscreenPixels = 512;
inline constexpr int kOffscreenScale = 2;

// The game reading its frame buffer back (sceGsExecStoreImage: the pause and vendor menus keep the
// frame they open over, and send it back into the frame each frame as image transfers). The port
// answers the read with the frame it drew, scaled to the chip's pixels (frame_pixel says where each
// pixel of a transfer is), and keeps that frame at full resolution here: a transfer back into the
// draw buffer whose pixels are ones it answered with is drawn from this texture instead.
struct FrameSnapshot {
    unsigned texture = 0;                   // GL, the frame as drawn (first row at the bottom)
    std::vector<std::uint64_t> answered;  // hash_bytes() of each block of pixels handed back
};
inline FrameSnapshot g_frame_snapshot;

// The draw buffer the games draw the frame in, read back and write into: 8 x 64 pixels wide, right
// after the display buffer. Where it is and how tall follow the display the game sets
// (set_draw_buffer_block): PAL Ratchet & Clank's 512-line display puts it at block 0x1000, 448
// lines tall; the 448-line display of its 60 Hz mode (and of the NTSC games) at 0xE00, 416 lines.
inline std::uint32_t g_draw_buffer_block = 0x1000;
inline int g_draw_buffer_lines = 448;
inline constexpr std::uint32_t kDrawBufferWidth = 8;
inline void set_draw_buffer_block(std::uint32_t block, int lines) {
    g_draw_buffer_block = block;
    g_draw_buffer_lines = lines;
}

// Where pixel (x, y) of a PSMCT32 rectangle at block `base`, `width` 64-pixel units wide, is in the
// draw buffer: through the chip's 64 x 32 pixel pages (the same pages whichever buffer width the
// transfer gives). False if the base is not on a page of the draw buffer.
bool frame_pixel(std::uint32_t base, std::uint32_t width, int x, int y, int& frame_x, int& frame_y);

class GifInterpreter {
public:
    GifInterpreter(TexturePool& textures, DirectConfig config = {});

    // A GIF packet: tags and their data, as the game sends to the chip
    // (PATH3, or DIRECT through VIF1). Returns false if it is malformed;
    // error() says where. An unfinished payload continues in the next call;
    // draws accumulate until clear().
    bool gif(std::span<const std::uint8_t> input);

    // The 2D layer's quads drawn natively: a marker in the 2D path (an A+D write to the NOP
    // register, kUiQuadMarker in the value's top 16 bits, the index in its low 24 bits; bits 24-29
    // name the tick that recorded it, which the frontend reads) draws quads[index] there, in the
    // context the packets before it set up.
    void set_ui_quads(std::span<const UiQuad> quads) { m_ui_quads = quads; }
    static constexpr std::uint64_t kUiQuadMarker = 0x4E55;
    // Mobys the game drew while rendering to a texture: a marker of the same form, kMobyMarker in
    // the top 16 bits, the mobys' count in bits 32-39 and the first's address in the low 32,
    // becomes a draw of no vertices (DirectDraw::mobys) in the target being drawn.
    static constexpr std::uint64_t kMobyMarker = 0x4D42;

    // A VIF1 stream carrying GIF data in DIRECT and DIRECTHL commands, with
    // the bookkeeping codes around them (NOP, FLUSH, STCYCL...). A command
    // that starts a VU program (MSCAL, UNPACK, MPG) is not the direct path:
    // the call returns false.
    bool vif(std::span<const std::uint8_t> stream);

    // One register write, as an A+D pair carries it.
    void write_register(std::uint8_t address, std::uint64_t value);

    // Forget this frame's vertices and draws; the registers keep their values.
    void clear();

    const std::vector<DirectVertex>& vertices() const { return m_vertices; }

    const std::vector<DirectDraw>& draws() const { return m_draws; }

    const std::string& error() const { return m_error; }

private:
    // A vertex as the chip holds it between the kick and the primitive.
    struct GsVertex {
        std::uint32_t x = 0, y = 0;  // 12.4 fixed point, before XYOFFSET
        std::uint32_t z = 0;
        std::uint8_t fog = 0xFF;
        std::uint8_t rgba[4] = {0x80, 0x80, 0x80, 0x80};
        float s = 0.0f, t = 0.0f, q = 1.0f;
        std::uint32_t u = 0, v = 0;  // 12.4 texel coordinates (PRIM.FST)
    };

    // The registers that come in two copies, one per drawing context.
    struct Context {
        gs::Tex0 tex0{};
        gs::Tex1 tex1{};
        gs::Clamp clamp{};
        gs::Xyoffset xyoffset{};
        gs::Scissor scissor{};
        gs::Alpha alpha{0, 1, 0, 1, 0};
        gs::Test test{};
        gs::Frame frame{};
        gs::Zbuf zbuf{};
    };

    void write_prim(std::uint64_t value);
    void kick(bool draw);
    void native_quad(const UiQuad& quad);
    std::span<const UiQuad> m_ui_quads{};
    bool m_glyph = false;  // native_quad: drawing a glyph
    bool m_in_native = false;  // native_quad: its own vertices
    void emit_triangle(
        const GsVertex& a, const GsVertex& b, const GsVertex& c, const GsVertex& colour_from
    );
    void emit_sprite(const GsVertex& a, const GsVertex& b);
    void emit_line(const GsVertex& a, const GsVertex& b);
    void emit_point(const GsVertex& a);
    void emit_quad(const DirectVertex corners[4]);
    // An image transfer into the draw buffer: its pixels drawn where the chip would store them.
    void draw_frame_upload(const ImageUpload& upload);
    DirectVertex convert(const GsVertex& v) const;
    void screen_position(const GsVertex& v, float& x, float& y) const;
    void ensure_draw();
    const gs::Prim& attributes() const;
    const Context& context() const;
    void image_data(std::span<const std::uint8_t> data);
    bool fail(std::string message);

    TexturePool& m_textures;
    DirectConfig m_config;

    // The frame buffers drawn to since the last clear, as texture base pointers (blocks): a
    // primitive textured from one of them reads back what was drawn (a full-screen blur, a
    // copy). Without render-to-texture those are left out rather than drawn with a wrong texture.
    std::vector<std::uint32_t> m_targets;
    // Frame buffers ever drawn to (block, width in 64 pixels), and copies the game made of parts
    // of them inside the chip: a texel (u, v) at `dbp` is the frame's pixel (u + x, v + y).
    struct FrameBuffer {
        std::uint32_t block, width;
    };
    struct FrameCopy {
        std::uint32_t dbp;
        int x, y;
    };
    std::vector<FrameBuffer> m_frame_buffers;
    std::vector<FrameCopy> m_frame_copies;
    // Off-screen targets ever drawn to (their GS block): a FRAME narrower than the screen, which the
    // game renders a panel into and then draws as a texture.
    std::vector<std::uint32_t> m_offscreen;
    // The off-screen target the current context draws into (its block), or 0.
    std::uint32_t offscreen_target() const;
    bool is_offscreen(const gs::Frame& f) const;  // a target that is not one of the frame buffers
    // Whether a texture at `tbp` is a frame buffer or a copy of one; if so, where its texel (0, 0)
    // is in the frame.
    bool frame_source(std::uint32_t tbp, int& x, int& y) const;

    Context m_context[2];
    gs::Prim m_prim{};
    gs::Prim m_prmode{};
    bool m_use_prim_attributes = true;  // PRMODECONT.AC
    gs::Texa m_texa{0, false, 0x80};
    std::uint32_t m_fog_colour = 0;
    GsVertex m_current;       // the vertex registers (RGBAQ, ST, UV, FOG)
    float m_packed_q = 1.0f;  // Q from a PACKED ST, taken by the next RGBAQ

    GsVertex m_queue[3];
    int m_queued = 0;

    // An image transfer in progress.
    gs::Bitbltbuf m_bitbltbuf{};
    gs::Trxpos m_trxpos{};
    gs::Trxreg m_trxreg{};
    ImageUpload m_upload;
    std::size_t m_upload_bytes = 0;  // expected; 0 when no transfer is open

    bool m_state_dirty = true;
    DirectState m_state;
    std::vector<DirectVertex> m_vertices;
    std::vector<DirectDraw> m_draws;
    std::string m_error;
    // A GIF tag and the part of its data that has come so far, finished by the next packet.
    std::vector<std::uint8_t> m_gif_pending;
    // Reading one VIF DIRECT's data (vif()): a register tag it cuts off is dropped, not carried.
    bool m_in_direct = false;
};

class DirectRenderer : public BucketRenderer {
public:
    // The packets in a bucket are GIF data, or a VIF1 stream carrying it.
    enum class Input {
        Gif,
        Vif
    };

    DirectRenderer(
        std::string name, Bucket bucket, DirectConfig config = {}, Input input = Input::Gif
    );
    ~DirectRenderer() override;

    bool init(RenderState& state, std::string& error) override;
    void render(const FrameInput& input, RenderState& state) override;
    void release() override;

    // As a component of another renderer: feed packets, then flush() draws
    // what was fed.
    bool submit(std::span<const std::uint8_t> packets);
    // `background`: the part of the frame before the world (DirectBackground), drawn without
    // depth writes, since the world it is under is drawn after it.
    void flush(RenderState& state, bool background = false);

    GifInterpreter& interpreter() { return *m_interpreter; }
    // The frame being drawn, for its moby markers (FrameInput::target_mobys).
    void set_frame_input(const FrameInput* input) { m_frame = input; }

private:
    const FrameInput* m_frame = nullptr;
    void apply(const DirectState& state, RenderState& render_state);
    void draw(const DirectDraw& draw, RenderState& render_state);
    bool m_background = false;  // flush(): the part before the world
    // flush(), on a target wider than the picture: the triangles being drawn are the whole
    // target's (what covered the whole screen), not the picture's in its middle.
    bool m_whole_target = true;

    DirectConfig m_config;
    Input m_input;
    std::unique_ptr<GifInterpreter> m_interpreter;
    Shader m_shader;
    unsigned m_vao = 0;
    unsigned m_vbo = 0;
    // The render target as drawn so far, copied for a draw that reads the frame (frame_source).
    unsigned m_frame_copy = 0;
    int m_frame_copy_width = 0;
    int m_frame_copy_height = 0;
    // The off-screen targets' GL side, by GS block: a square of kOffscreenPixels chip pixels.
    struct Offscreen {
        unsigned framebuffer = 0;
        unsigned texture = 0;
        unsigned depth = 0;
    };
    std::vector<std::pair<std::uint32_t, Offscreen>> m_offscreen_targets;
    const Offscreen& offscreen(std::uint32_t block);
    std::size_t m_vbo_bytes = 0;
    std::string m_last_error;

    struct Uniforms {
        int textured, tcc, tfx, tex_alpha_scale, fog_enable, fog_colour, alpha_test, alpha_ref,
            alpha_keep_failing, distance_field, tex_T1, pixel_offset;
    } m_uniforms{};
};

// The part of a DirectRenderer's frame that comes before the world (FrameInput::
// direct_before_world), drawn by that renderer when this one's turn comes: added ahead of the world
// renderers, the DirectRenderer itself after them. Its registers carry on from one part to the next.
class DirectBackground : public BucketRenderer {
public:
    explicit DirectBackground(DirectRenderer& direct)
        : BucketRenderer(direct.name() + " (before the world)", direct.bucket()),
          m_direct(direct) {}

    void render(const FrameInput& input, RenderState& state) override {
        if (!input.direct_before_world.empty()) {
            m_direct.interpreter().set_ui_quads(input.ui_quads);
            m_direct.set_frame_input(&input);
            m_direct.submit(input.direct_before_world);
            m_direct.flush(state, true);
        }
    }

private:
    DirectRenderer& m_direct;
};

}  // namespace openrac::renderer
