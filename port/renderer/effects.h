// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The world's effect quads (FrameInput::effects): what the game's draw callbacks send through its
// quad routine (glows, beams, the space scenes' pictures, shadows), drawn with the GS state the
// game gave each one: its texture from TEX0, filtering from TEX1, wrapping from CLAMP, blending
// from ALPHA, the texture function and alpha as the direct renderer does (direct.frag). Depth is
// tested against the world and not written, as the game's effect list draws. The strips the game
// sends to its VU1 strip program (FrameInput::strips) are drawn first, the same way.

#pragma once

#include <string>

#include "renderer/renderer.h"
#include "renderer/shader.h"

namespace openrac::renderer {

class EffectRenderer : public BucketRenderer {
public:
    explicit EffectRenderer(std::string name = "effects") : BucketRenderer(std::move(name), Bucket::Particles) {}

    bool init(RenderState& state, std::string& error) override;
    void render(const FrameInput& input, RenderState& state) override;
    void release() override;

private:
    // Binds TEX0's texture with TEX1's filtering and CLAMP's wrapping, and sets ALPHA's blend.
    // False when the pool has nothing at TEX0's block (the draw is left out, not the placeholder).
    bool apply(std::uint64_t tex0, std::uint64_t tex1, std::uint64_t clamp, std::uint64_t alpha, RenderState& state,
               std::uint32_t tbp = 0, std::uint32_t cbp = 0);

    struct Vertex {
        float x, y, z;
        float s, t;
        std::uint8_t r, g, b, a;
    };

    Shader m_shader;
    int m_view_projection = -1;
    int m_textured = -1, m_tcc = -1, m_tfx = -1, m_tex_alpha_scale = -1;
    int m_fog_enable = -1, m_alpha_test = -1;
    unsigned m_vao = 0, m_vbo = 0;
};

}  // namespace openrac::renderer
