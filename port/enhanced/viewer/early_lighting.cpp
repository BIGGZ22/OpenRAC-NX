// SPDX-License-Identifier: GPL-3.0-or-later
#include "enhanced/viewer/early_lighting.h"

#include "common/stats.h"
#include "enhanced/viewer/probe_lighting.h"
#include "enhanced_viewer_shaders.h"
#include "renderer/gl.h"
#include "renderer/world/gl_world.h"

namespace openrac::lighting {
using namespace openrac::gl;
namespace wgl = renderer::world::gl;

bool EarlyLighting::supported() {
    static const bool& off = stats::probe(
        "OPENRAC_EARLY_LIGHTING_OFF", "Reference: full-resolution GI and cascades (reload level)"
    );
    return enhanced_rendering_available() && !off && glDispatchCompute && glMemoryBarrier;
}

bool EarlyLighting::render(
    unsigned depth,
    const renderer::FrameInput& input,
    const renderer::RenderTarget& target,
    const std::function<void(const renderer::Shader&)>& bind_lighting,
    std::string& error
) {
    m_ready = false;
    const int width = target.width, height = target.height;
    if (m_failed || !supported() || !depth || width < 2 || height < 2
        || input.camera.projection[11] != -1.0f || input.camera.projection[0] == 0
        || input.camera.projection[5] == 0) {
        return false;
    }
    const std::size_t cells = std::size_t((width + 1) / 2) * ((height + 1) / 2);
    GLint limit = 0;
    glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &limit);
    if (cells * 2 > std::size_t(limit)) {
        return false;
    }
    if (!m_compute.id()) {
        std::string source = shaders::early_lighting_comp;
        source = probe_shader_source(std::move(source));
        source.insert(
            source.find('\n') + 1, "#define OPENRAC_PROBE_PIXEL early_pixel\nivec2 early_pixel;\n"
        );
        if (!m_compute.build_compute("early GI and cascades", source, error)) {
            m_failed = true;
            return false;
        }
    }
    if (!m_buffers[0]) {
        glGenBuffers(2, m_buffers.data());
        glGenTextures(2, m_textures.data());
    }
    for (unsigned i = 0; i < 2; ++i) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffers[i]);
        if (width != m_width || height != m_height) {
            glBufferData(GL_SHADER_STORAGE_BUFFER, cells * (i ? 8 : 4), nullptr, GL_DYNAMIC_DRAW);
        }
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9 + i, m_buffers[i]);
        glActiveTexture(GL_TEXTURE0 + 16 + i);
        glBindTexture(wgl::GL_TEXTURE_BUFFER, m_textures[i]);
        glTexBuffer(wgl::GL_TEXTURE_BUFFER, i ? wgl::GL_R32UI : GL_RGBA8, m_buffers[i]);
    }
    m_compute.use();
    bind_lighting(m_compute);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depth);
    glUniform1i(m_compute.uniform("early_depth"), 0);
    glUniformMatrix4fv(
        m_compute.uniform("early_projection"), 1, GL_FALSE, input.camera.projection.data()
    );
    glUniformMatrix4fv(m_compute.uniform("early_view"), 1, GL_FALSE, input.camera.view.data());
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    {
        const stats::Scope pass("early GI and cascades", stats::kGpu);
        glDispatchCompute(unsigned((width + 15) / 16), unsigned((height + 15) / 16), 1);
    }
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    m_width = width;
    m_height = height;
    m_clip = renderer::multiply(input.camera.projection, input.camera.view);
    m_frame = input.frame;
    m_target = target.framebuffer;
    m_ready = true;
    glBindTexture(GL_TEXTURE_2D, 0);
    stats::gauge("early lighting bytes", double(cells * 12));
    stats::gauge("early lighting texels", double(cells));
    return true;
}

void EarlyLighting::bind(
    const renderer::Shader& shader,
    const renderer::FrameInput& input,
    const renderer::RenderState& state
) const {
    const bool active = matches(input, state);
    glUniform1i(shader.uniform("early_lighting_enabled"), active ? 1 : 0);
    if (!active) {
        return;
    }
    glUniform2f(shader.uniform("early_extent"), float(m_width), float(m_height));
    glUniformMatrix4fv(
        shader.uniform("early_projection"), 1, GL_FALSE, input.camera.projection.data()
    );
    glUniform1i(shader.uniform("early_values"), 16);
    glUniform1i(shader.uniform("early_guides"), 17);
    for (unsigned i = 0; i < 2; ++i) {
        glActiveTexture(GL_TEXTURE0 + 16 + i);
        glBindTexture(wgl::GL_TEXTURE_BUFFER, m_textures[i]);
    }
    glActiveTexture(GL_TEXTURE0);
}

bool EarlyLighting::matches(const renderer::FrameInput& input, const renderer::RenderState& state)
    const {
    GLint viewport[4]{};
    glGetIntegerv(GL_VIEWPORT, viewport);
    return state.early_lighting && m_ready && m_frame == input.frame
           && m_target == state.target.framebuffer && viewport[0] == 0 && viewport[1] == 0
           && viewport[2] == m_width && viewport[3] == m_height && m_width == state.target.width
           && m_height == state.target.height
           && m_clip == renderer::multiply(input.camera.projection, input.camera.view);
}

void EarlyLighting::release() {
    m_compute.release();
    if (m_buffers[0]) {
        glDeleteBuffers(2, m_buffers.data());
    }
    if (m_textures[0]) {
        glDeleteTextures(2, m_textures.data());
    }
    m_buffers = {};
    m_textures = {};
    m_width = m_height = 0;
    m_ready = false;
    m_failed = false;
}
}  // namespace openrac::lighting
