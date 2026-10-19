// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The world's effect quads (FrameInput::effects): what the game's draw callbacks send through its
// quad routine (glows, beams, the space scenes' pictures, shadows), drawn with the GS state the
// game gave each one: its texture from TEX0, filtering from TEX1, wrapping from CLAMP, blending
// from ALPHA, the texture function and alpha as the direct renderer does (direct.frag). Depth is
// tested against the world and not written, as the game's effect list draws. The strips the game
// sends to its VU1 strip program (FrameInput::strips) are drawn first, the same way.
//
// The liquids (EffectPart::Liquids) are a pass of their own after the ties: the strips the game
// draws as a solid surface (ALPHA 0x64, (Cs - Cd) FIX + Cd, with FIX above 0x60, which it pairs
// with a Z-writing TEST: the liquid grid's seas and lava), with depth written. Retail draws them
// from the draw callback list it runs after the ties, before the mobys; drawn with the effects,
// after the mobys, a sea covered the mobys' glow faces and blended parts in front of it (Umbris's
// lamps and the bolts' glints as black patches).

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "renderer/renderer.h"
#include "renderer/shader.h"

namespace openrac::renderer {

// Which of the frame's strips and quads a pass draws.
enum class EffectPart : std::uint8_t {
    All,      // every strip, then every quad
    Liquids,  // only the solid strips (liquid_strip), with depth written
    Rest,     // the strips that are not, then every quad
};

// A strip drawn as a solid surface: ALPHA 0x64 ((Cs - Cd) FIX + Cd) with FIX above 0x60.
bool liquid_strip(std::uint64_t alpha);

class EffectRenderer : public BucketRenderer {
public:
    explicit EffectRenderer(std::string name = "effects", EffectPart part = EffectPart::All,
                            Bucket bucket = Bucket::Particles)
        : BucketRenderer(std::move(name), bucket), m_part(part) {}

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

    // Whether this pass draws `strip` (m_part).
    bool draws(const EffectStrip& strip) const;

    EffectPart m_part = EffectPart::All;
    Shader m_shader;
    int m_view_projection = -1;
    int m_textured = -1, m_tcc = -1, m_tfx = -1, m_tex_alpha_scale = -1;
    int m_fog_enable = -1, m_alpha_test = -1;
    unsigned m_vao = 0, m_vbo = 0;
    std::vector<Vertex> m_vertices;  // the frame's triangles, kept for their storage
};

}  // namespace openrac::renderer
