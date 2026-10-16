// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/effects.h"

#include <cstddef>
#include <set>
#include <vector>

#include "common/log.h"

#include "renderer/direct.h"
#include "renderer/gl.h"
#include "renderer/gs.h"
#include "renderer_shaders.h"

namespace openrac::renderer {

using namespace gl;

bool EffectRenderer::init(RenderState& state, std::string& error) {
    (void)state;
    if (!m_shader.build("effect", shaders::effect_vert, shaders::direct_frag, error)) {
        return false;
    }
    m_view_projection = m_shader.uniform("view_projection");
    m_textured = m_shader.uniform("textured");
    m_tcc = m_shader.uniform("tcc");
    m_tfx = m_shader.uniform("tfx");
    m_tex_alpha_scale = m_shader.uniform("tex_alpha_scale");
    m_fog_enable = m_shader.uniform("fog_enable");
    m_alpha_test = m_shader.uniform("alpha_test");

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    constexpr auto stride = static_cast<GLsizei>(sizeof(Vertex));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(Vertex, x)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(Vertex, s)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, reinterpret_cast<const void*>(offsetof(Vertex, r)));
    glBindVertexArray(0);
    return true;
}

void EffectRenderer::render(const FrameInput& input, RenderState& state) {
    if (input.effects.empty() && input.strips.empty()) {
        return;
    }
    m_shader.use();
    const Mat4 view_projection = multiply(input.camera.projection, input.camera.view);
    glUniformMatrix4fv(m_view_projection, 1, GL_FALSE, view_projection.data());
    glUniform1i(m_fog_enable, 0);
    glUniform1i(m_alpha_test, 0);
    glUniform1i(m_textured, 1);
    glBindFramebuffer(GL_FRAMEBUFFER, state.target.framebuffer);
    glViewport(0, 0, state.target.width, state.target.height);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);  // reversed depth
    glDepthMask(GL_FALSE);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glActiveTexture(GL_TEXTURE0);

    std::vector<Vertex> vertices(4);
    for (const EffectStrip& strip : input.strips) {
        if (strip.count < 3 || static_cast<std::size_t>(strip.first) + strip.count > input.strip_vertices.size()) {
            continue;
        }
        for (int u = 0; u < strip.upload_count; ++u) {
            const std::size_t i = static_cast<std::size_t>(strip.uploads + u);
            if (strip.uploads >= 0 && i < input.effect_uploads.size()) {
                state.textures.upload(input.effect_uploads[i]);
            }
        }
        apply(strip.tex0, strip.tex1, strip.clamp, strip.alpha, state);
        vertices.resize(strip.count);
        for (std::uint32_t k = 0; k < strip.count; ++k) {
            const EffectVertex& e = input.strip_vertices[strip.first + k];
            vertices[k] = {e.x, e.y, e.z, e.s, e.t, static_cast<std::uint8_t>(e.rgba & 0xFF),
                           static_cast<std::uint8_t>((e.rgba >> 8) & 0xFF), static_cast<std::uint8_t>((e.rgba >> 16) & 0xFF),
                           static_cast<std::uint8_t>(e.rgba >> 24)};
        }
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(),
                     GL_STREAM_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(strip.count));
        state.stats.draw_calls++;
        state.stats.triangles += strip.count - 2;
    }
    vertices.resize(4);
    // Every quad's texture before any quad is drawn. A texture's upload rides on the first quad that
    // named it (frontend::particle_quads hands out the pool keys per frame, in pool order), but the
    // quads are drawn back to front: one drawn before that quad sampled what the key held last frame,
    // another texture, and the nanotech crates' orbs came out as opaque white quads.
    // Refactor note: the uploads could travel as a list of their own instead of riding on quads.
    for (const EffectQuad& q : input.effects) {
        if (q.uploads >= 0 && static_cast<std::size_t>(q.uploads) + 1 < input.effect_uploads.size()) {
            state.textures.upload(input.effect_uploads[static_cast<std::size_t>(q.uploads)]);
            state.textures.upload(input.effect_uploads[static_cast<std::size_t>(q.uploads) + 1]);
        }
    }
    for (const EffectQuad& q : input.effects) {
        apply(q.tex0, q.tex1, q.clamp, q.alpha, state, q.tbp, q.cbp);

        for (int k = 0; k < 4; ++k) {
            Vertex& v = vertices[static_cast<std::size_t>(k)];
            v.x = q.corner[k][0];
            v.y = q.corner[k][1];
            v.z = q.corner[k][2];
            v.s = q.st[k][0];
            v.t = q.st[k][1];
            v.r = static_cast<std::uint8_t>(q.rgba[k] & 0xFF);
            v.g = static_cast<std::uint8_t>((q.rgba[k] >> 8) & 0xFF);
            v.b = static_cast<std::uint8_t>((q.rgba[k] >> 16) & 0xFF);
            v.a = static_cast<std::uint8_t>(q.rgba[k] >> 24);
        }
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(),
                     GL_STREAM_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        state.stats.draw_calls++;
        state.stats.triangles += 2;
    }
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

void EffectRenderer::apply(std::uint64_t tex0_bits, std::uint64_t tex1_bits, std::uint64_t clamp_bits,
                           std::uint64_t alpha_bits, RenderState& state, std::uint32_t tbp, std::uint32_t cbp) {
    const gs::Texa texa{0, false, 0x80};
    gs::Tex0 tex0 = gs::Tex0::decode(tex0_bits);
    if (tbp != 0) {
        tex0.tbp0 = tbp;
        tex0.cbp = cbp;
    }
    const gs::Tex1 tex1 = gs::Tex1::decode(tex1_bits);
    const gs::Clamp clamp = gs::Clamp::decode(clamp_bits);
    const TextureHandle texture = state.textures.resolve(tex0, texa);
    static std::set<std::uint64_t> seen;
    if (seen.insert(tex0_bits).second) {
        const Rgba8Image& image = state.textures.image(texture);
        log::debug("effect texture {:#x}: block {:#x}, {}x{}, {}{}", tex0_bits, tex0.tbp0, image.width, image.height,
                   state.textures.name(texture), texture == state.textures.placeholder() ? " (placeholder)" : "");
    }
    glBindTexture(GL_TEXTURE_2D, state.textures.gl_texture(texture));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(tex1.mmag ? GL_LINEAR : GL_NEAREST));
    const bool linear_min = tex1.mmin == 1 || tex1.mmin == 4 || tex1.mmin == 5;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(linear_min ? GL_LINEAR : GL_NEAREST));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    static_cast<GLint>(clamp.wms == gs::Wrap::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    static_cast<GLint>(clamp.wmt == gs::Wrap::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE));
    glUniform1i(m_tcc, tex0.tcc ? 1 : 0);
    glUniform1i(m_tfx, static_cast<GLint>(tex0.tfx));
    glUniform1f(m_tex_alpha_scale, state.textures.alpha_scale(texture) == AlphaScale::Full ? 0.5f : 1.0f);
    const GlBlend b = translate_blend(gs::Alpha::decode(alpha_bits));
    glEnable(GL_BLEND);
    glBlendEquation(b.equation);
    glBlendFuncSeparate(b.src, b.dst, GL_ONE, GL_ZERO);
    glBlendColor(0.0f, 0.0f, 0.0f, b.constant);
}

void EffectRenderer::release() {
    m_shader.release();
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
}

}  // namespace openrac::renderer
