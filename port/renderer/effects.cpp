// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/effects.h"

#include <cstddef>
#include <optional>
#include <set>
#include <span>
#include <vector>

#include "common/log.h"
#include "common/stats.h"
#include "renderer/direct.h"
#include "renderer/gl.h"
#include "renderer/gs.h"
#include "renderer_shaders.h"

namespace openrac::renderer {

using namespace gl;

bool liquid_strip(std::uint64_t alpha) {
    return (alpha & 0xFF) == 0x64 && ((alpha >> 32) & 0xFF) > 0x60;
}

bool EffectRenderer::draws(const EffectStrip& strip) const {
    switch (m_part) {
        case EffectPart::Liquids:
            return liquid_strip(strip.alpha);
        case EffectPart::Rest:
            return !liquid_strip(strip.alpha);
        case EffectPart::All:
            break;
    }
    return true;
}

bool EffectRenderer::init(RenderState& state, std::string& error) {
    (void)state;
    std::string fragment(shaders::direct_frag);
    fragment.insert(fragment.find('\n') + 1, "#define OPENRAC_UPSCALED_TEXTURE_COLOURS 1\n");
    if (!m_shader.build("effect", shaders::effect_vert, fragment, error)) {
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
    const bool quads = m_part != EffectPart::Liquids && !input.effects.empty();
    bool any = quads;
    for (const EffectStrip& strip : input.strips) {
        any = any || draws(strip);
    }
    if (!any) {
        return;
    }
    m_shader.use();
    m_texture_filter = input.texture_filter;
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
    glDepthMask(m_part == EffectPart::Liquids ? GL_TRUE : GL_FALSE);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glActiveTexture(GL_TEXTURE0);
    m_applied = AppliedState{};

    static const bool& upload_reference = stats::probe(
        "OPENRAC_VK_EFFECT_UPLOAD_REUSE_OFF", "Reference: copy and hash every effect texture upload"
    );
    const bool reuse_uploads = gl::is_vulkan() && !upload_reference;
    std::uint64_t reused_uploads = 0, reused_bytes = 0;
    const auto upload = [&](const ImageUpload& image) {
        if (reuse_uploads) {
            if (state.textures.upload_cached(image)) {
                ++reused_uploads;
                reused_bytes += image.data.size();
            }
        } else {
            state.textures.upload(image);
        }
    };

    // Every strip and quad as triangles in one buffer, sent once; consecutive ones with the same GS
    // state are then one draw. A frame has hundreds to thousands of them (particles, glows), and
    // a draw each, with its own buffer and state, cost more than the rest of the frame on a phone.
    // The triangles keep their order, so what is blended over what does not change.
    m_vertices.clear();
    const auto vertex = [](const EffectVertex& e) {
        return Vertex{e.x, e.y, e.z, e.s, e.t, static_cast<std::uint8_t>(e.rgba & 0xFF),
                      static_cast<std::uint8_t>((e.rgba >> 8) & 0xFF), static_cast<std::uint8_t>((e.rgba >> 16) & 0xFF),
                      static_cast<std::uint8_t>(e.rgba >> 24)};
    };
    for (const EffectStrip& strip : input.strips) {
        if (strip.count < 3 || static_cast<std::size_t>(strip.first) + strip.count > input.strip_vertices.size()
            || !draws(strip)) {
            continue;
        }
        for (std::uint32_t k = 0; k + 2 < strip.count; ++k) {
            for (std::uint32_t c = 0; c < 3; ++c) {
                m_vertices.push_back(vertex(input.strip_vertices[strip.first + k + c]));
            }
        }
    }
    const std::size_t quads_from = m_vertices.size();
    for (const EffectQuad& q : quads ? input.effects : std::span<const EffectQuad>{}) {
        // The strip's two triangles, 0-1-2 and 1-2-3 (the faces are not culled).
        for (const int k : {0, 1, 2, 1, 2, 3}) {
            m_vertices.push_back({q.corner[k][0], q.corner[k][1], q.corner[k][2], q.st[k][0], q.st[k][1],
                                  static_cast<std::uint8_t>(q.rgba[k] & 0xFF),
                                  static_cast<std::uint8_t>((q.rgba[k] >> 8) & 0xFF),
                                  static_cast<std::uint8_t>((q.rgba[k] >> 16) & 0xFF),
                                  static_cast<std::uint8_t>(q.rgba[k] >> 24)});
        }
    }
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_vertices.size() * sizeof(Vertex)), m_vertices.data(),
                 GL_STREAM_DRAW);

    // The draw being gathered: its first vertex and count, and the GS state it was set up with.
    struct Batch {
        GLint first = 0;
        GLsizei count = 0;
    } batch;
    struct State {
        std::uint64_t tex0, tex1, clamp, alpha;
        std::uint32_t tbp, cbp;
        bool operator==(const State&) const = default;
    };
    std::optional<State> set;
    const auto flush = [&] {
        if (batch.count > 0) {
            glDrawArrays(GL_TRIANGLES, batch.first, batch.count);
            state.stats.draw_calls++;
            state.stats.triangles += static_cast<std::uint64_t>(batch.count / 3);
        }
        batch.count = 0;
    };
    // Adds `count` vertices from `first` to the draw, with this state; false when the state has
    // no texture in the pool (apply).
    bool drawable = true;
    const auto add = [&](const State& wanted, GLint first, GLsizei count, bool must_have_texture) {
        if (!set || !(*set == wanted)) {
            flush();
            drawable = apply(wanted.tex0, wanted.tex1, wanted.clamp, wanted.alpha, state, wanted.tbp, wanted.cbp);
            set = wanted;
        }
        if (must_have_texture && !drawable) {
            return;
        }
        if (batch.count > 0 && batch.first + batch.count != first) {
            flush();
        }
        if (batch.count == 0) {
            batch.first = first;
        }
        batch.count += count;
    };

    GLint at = 0;
    for (const EffectStrip& strip : input.strips) {
        if (strip.count < 3 || static_cast<std::size_t>(strip.first) + strip.count > input.strip_vertices.size()
            || !draws(strip)) {
            continue;
        }
        // An image put in the pool may replace what an earlier strip of the draw samples: that one
        // is drawn first, and the state is set again after.
        if (strip.uploads >= 0 && strip.upload_count > 0) {
            flush();
            set.reset();
            for (int u = 0; u < strip.upload_count; ++u) {
                const std::size_t i = static_cast<std::size_t>(strip.uploads + u);
                if (i < input.effect_uploads.size()) {
                    upload(input.effect_uploads[i]);
                }
            }
        }
        const auto count = static_cast<GLsizei>(3 * (strip.count - 2));
        add({strip.tex0, strip.tex1, strip.clamp, strip.alpha, 0, 0}, at, count, false);
        at += count;
    }
    flush();
    // Every quad's texture before any quad is drawn. A texture's upload rides on the first quad that
    // named it (frontend::particle_quads hands out the pool keys per frame, in pool order), but the
    // quads are drawn back to front: one drawn before that quad sampled what the key held last frame,
    // another texture, and the nanotech crates' orbs came out as opaque white quads.
    // Refactor note: the uploads could travel as a list of their own instead of riding on quads.
    for (const EffectQuad& q : quads ? input.effects : std::span<const EffectQuad>{}) {
        if (q.uploads >= 0 && static_cast<std::size_t>(q.uploads) + 1 < input.effect_uploads.size()) {
            upload(input.effect_uploads[static_cast<std::size_t>(q.uploads)]);
            upload(input.effect_uploads[static_cast<std::size_t>(q.uploads) + 1]);
        }
    }
    set.reset();
    at = static_cast<GLint>(quads_from);
    for (const EffectQuad& q : quads ? input.effects : std::span<const EffectQuad>{}) {
        add({q.tex0, q.tex1, q.clamp, q.alpha, q.tbp, q.cbp}, at, 6, true);
        at += 6;
    }
    flush();
    stats::gauge("effects uploads reused", double(reused_uploads));
    stats::gauge("effects upload bytes reused", double(reused_bytes));
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

bool EffectRenderer::apply(std::uint64_t tex0_bits, std::uint64_t tex1_bits, std::uint64_t clamp_bits,
                           std::uint64_t alpha_bits, RenderState& state, std::uint32_t tbp, std::uint32_t cbp) {
    const gs::Texa texa{0, false, 0x80};
    gs::Tex0 tex0 = gs::Tex0::decode(tex0_bits);
    if (tbp != 0) {
        tex0.tbp0 = tbp;
        tex0.cbp = cbp;
    }
    const TextureHandle texture = state.textures.resolve(tex0, texa);
    if (log::level() <= log::Level::Debug) {
        static std::set<std::uint64_t> seen;
        if (seen.insert(tex0_bits).second) {
            const Rgba8Image& image = state.textures.image(texture);
            log::debug("effect texture {:#x}: block {:#x}, {}x{}, {}{}", tex0_bits, tex0.tbp0, image.width, image.height,
                       state.textures.name(texture), texture == state.textures.placeholder() ? " (placeholder)" : "");
        }
    }
    if (texture != m_applied.texture || clamp_bits != m_applied.clamp_bits) {
        const auto wrap = gs::Clamp::decode(clamp_bits);
        state.textures.bind_upscaled_texture_colours(
            texture,
            m_shader,
            m_texture_filter,
            wrap.wms == gs::Wrap::Repeat,
            wrap.wmt == gs::Wrap::Repeat
        );
    }
    m_applied.texture = texture;
    if (tex1_bits != m_applied.tex1_bits || clamp_bits != m_applied.clamp_bits) {
        const gs::Tex1 tex1 = gs::Tex1::decode(tex1_bits);
        const gs::Clamp clamp = gs::Clamp::decode(clamp_bits);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(tex1.mmag ? GL_LINEAR : GL_NEAREST));
        const bool linear_min = tex1.mmin == 1 || tex1.mmin == 4 || tex1.mmin == 5;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(linear_min ? GL_LINEAR : GL_NEAREST));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                        static_cast<GLint>(clamp.wms == gs::Wrap::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                        static_cast<GLint>(clamp.wmt == gs::Wrap::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE));
        m_applied.tex1_bits = tex1_bits;
        m_applied.clamp_bits = clamp_bits;
    }
    const int tcc_val = tex0.tcc ? 1 : 0;
    if (tcc_val != m_applied.tcc) {
        glUniform1i(m_tcc, tcc_val);
        m_applied.tcc = tcc_val;
    }
    const int tfx_val = static_cast<GLint>(tex0.tfx);
    if (tfx_val != m_applied.tfx) {
        glUniform1i(m_tfx, tfx_val);
        m_applied.tfx = tfx_val;
    }
    const float scale = state.textures.alpha_scale(texture) == AlphaScale::Full ? 0.5f : 1.0f;
    if (scale != m_applied.alpha_scale) {
        glUniform1f(m_tex_alpha_scale, scale);
        m_applied.alpha_scale = scale;
    }
    if (alpha_bits != m_applied.alpha_bits) {
        const GlBlend b = translate_blend(gs::Alpha::decode(alpha_bits));
        glEnable(GL_BLEND);
        glBlendEquation(b.equation);
        glBlendFuncSeparate(b.src, b.dst, GL_ONE, GL_ZERO);
        glBlendColor(0.0f, 0.0f, 0.0f, b.constant);
        m_applied.alpha_bits = alpha_bits;
    }
    return texture != state.textures.placeholder();
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
