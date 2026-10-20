// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#include "renderer/shadows.h"

#include "renderer/gl.h"
#include "renderer_shaders.h"

namespace openrac::renderer {
using namespace openrac::gl;

bool ShadowRenderer::init(RenderState&, std::string& error) {
    if (!m_volume.build("shadow volumes", shaders::shadow_vert, shaders::shadow_frag, error)
        || !m_composite.build(
            "shadow composite", shaders::shadow_composite_vert, shaders::shadow_frag, error
        )) {
        return false;
    }
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_buffer);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), nullptr);
    glBindVertexArray(0);
    return true;
}

void ShadowRenderer::render(const FrameInput& input, RenderState& state) {
    const auto vertices =
        shadow_volumes(input.shadow_capsules, input.shadow_extrusion, input.shadow_heights);
    if (vertices.empty()) {
        return;
    }
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(Vec3)),
        vertices.data(),
        GL_STREAM_DRAW
    );
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glDepthMask(GL_FALSE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xff);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 0, 0xff);
    // Depth fail (reversed depth too): count the closed surfaces behind the
    // receiver. Overlapping capsules are one shadow, composited only once.
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
    m_volume.use();
    const auto vp = multiply(input.camera.projection, input.camera.view);
    glUniformMatrix4fv(m_volume.uniform("view_projection"), 1, GL_FALSE, vp.data());
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0);
    glStencilFunc(GL_NOTEQUAL, 0, 0xff);
    glStencilOpSeparate(GL_FRONT_AND_BACK, GL_KEEP, GL_KEEP, GL_KEEP);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
    m_composite.use();
    // Retail submit_graphics_setup_command_stream uses ALPHA FIX=32, divisor 128.
    glUniform1f(m_composite.uniform("opacity"), 32.0f / 128.0f);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    state.stats.draw_calls += 2;
    state.stats.triangles += vertices.size() / 3 + 1;

    glDisable(GL_STENCIL_TEST);
    glStencilMask(0xff);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glDepthMask(GL_TRUE);
    glBindVertexArray(0);
}

void ShadowRenderer::release() {
    m_volume.release();
    m_composite.release();
    if (m_buffer) {
        glDeleteBuffers(1, &m_buffer);
    }
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
    }
    m_buffer = m_vao = 0;
}
}  // namespace openrac::renderer
