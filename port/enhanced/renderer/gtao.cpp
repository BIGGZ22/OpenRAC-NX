// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#include "enhanced/renderer/gtao.h"

#include <algorithm>

#include "common/log.h"
#include "common/stats.h"
#include "renderer/gl.h"
#include "renderer/renderer.h"
#include "renderer/world/gl_world.h"
#include "renderer_shaders.h"

namespace openrac::renderer {
using namespace openrac::gl;

bool Gtao::prepare(int width, int height, int scale, bool visibility, bool copy_depth) {
    if (!enhanced_rendering_available()) {
        return false;
    }
    if (m_failed || !glDispatchCompute || !glMemoryBarrier) {
        return false;
    }
    std::string error;
    if (m_vao == 0) {
        std::string evaluate(shaders::gtao_evaluate_comp);
        std::string composite(shaders::gtao_composite_frag);
        evaluate.insert(evaluate.find('\n') + 1, shaders::gtao_common_glsl);
        composite.insert(composite.find('\n') + 1, shaders::gtao_common_glsl);
        std::string performance = evaluate;
        performance.insert(performance.find('\n') + 1, "#define GTAO_REFINEMENT_STEPS 3\n");
        if (!m_evaluate_performance.build_compute("GTAO performance horizons", performance, error)
            || !m_evaluate.build_compute("GTAO horizons", evaluate, error)
            || !m_composite.build("GTAO filter", shaders::gtao_vert, composite, error)) {
            log::error("GTAO: {}", error);
            m_failed = true;
            return false;
        }
        glGenVertexArrays(1, &m_vao);
    }
    if (copy_depth && (m_depth.width() != width || m_depth.height() != height)
        && !m_depth.create(width, height, error, 1, true)) {
        log::error("GTAO: {}", error);
        m_failed = true;
        return false;
    }
    const int ao_width = (width + scale - 1) / scale;
    const int ao_height = (height + scale - 1) / scale;
    if (m_ao_width != ao_width || m_ao_height != ao_height) {
        GLint limit = 0;
        glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &limit);
        const std::size_t cells = std::size_t(ao_width) * ao_height;
        if (cells > std::size_t(limit)) {
            return false;
        }
        if (!m_ao_buffer) {
            glGenBuffers(1, &m_ao_buffer);
            glGenTextures(1, &m_ao_texture);
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ao_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, cells * 4, nullptr, GL_DYNAMIC_DRAW);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(world::gl::GL_TEXTURE_BUFFER, m_ao_texture);
        glTexBuffer(world::gl::GL_TEXTURE_BUFFER, GL_RGBA8, m_ao_buffer);
        m_ao_width = ao_width;
        m_ao_height = ao_height;
    }
    if (visibility && (m_visibility.width() != width || m_visibility.height() != height)
        && !m_visibility.create(width, height, error)) {
        log::error("GTAO: {}", error);
        m_failed = true;
        return false;
    }
    return true;
}

bool Gtao::render(
    const FrameInput& input, const RenderTarget& target, bool apply_to_colour, bool evaluate_only
) {
    m_sampled_depth = 0;
    if (!enhanced_rendering_available()) {
        return false;
    }
    // Both game and viewer use this perspective form, including off-centre
    // projections. Menus and orthographic targets have no world AO.
    const Mat4& p = input.camera.projection;
    const auto& options = input.gtao_settings;
    const int scale = options.half_resolution ? 2 : 1;
    if (target.width < 2 || target.height < 2 || p[11] != -1.0f || p[0] == 0 || p[5] == 0
        || options.strength <= 0.0f) {
        return false;
    }
    // Sampling an attachment while drawing back into it would be framebuffer
    // feedback. Direct reads are restricted to compute/offscreen consumers.
    const bool direct_depth = target.depth_texture != 0 && !apply_to_colour;
    if (!prepare(
            target.width, target.height, scale, !apply_to_colour && !evaluate_only, !direct_depth
        )) {
        glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
        glViewport(0, 0, target.width, target.height);
        return false;
    }
    const stats::Scope pass("gtao");
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    m_sampled_depth = direct_depth ? target.depth_texture : m_depth.depth_texture();
    if (!direct_depth) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, target.framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_depth.id());
        const stats::Scope depth("GTAO depth copy", stats::kGpu);
        glBlitFramebuffer(
            0,
            0,
            target.width,
            target.height,
            0,
            0,
            target.width,
            target.height,
            GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
            GL_NEAREST
        );
    }
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glBindVertexArray(m_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_sampled_depth);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, m_ao_buffer);
    {
        const stats::Scope horizons("GTAO horizons", stats::kGpu);
        Shader& evaluate = options.quality == GtaoQuality::Performance
                               ? m_evaluate_performance : m_evaluate;
        evaluate.use();
        glUniform1i(evaluate.uniform("tex_T0"), 0);
        glUniformMatrix4fv(evaluate.uniform("projection"), 1, GL_FALSE, p.data());
        glUniform1f(evaluate.uniform("radius"), std::clamp(options.radius, 0.05f, 8.0f));
        glUniform1i(evaluate.uniform("evaluation_scale"), scale);
        glUniform1i(evaluate.uniform("slice_count"), std::clamp(options.slices, 1, 12));
        glUniform1i(evaluate.uniform("step_count"), std::clamp(options.steps, 4, 24));
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        glDispatchCompute(unsigned((m_ao_width + 7) / 8), unsigned((m_ao_height + 7) / 8), 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    }

    if (!evaluate_only) {
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(world::gl::GL_TEXTURE_BUFFER, m_ao_texture);
        glBindFramebuffer(GL_FRAMEBUFFER, apply_to_colour ? target.framebuffer : m_visibility.id());
        glViewport(0, 0, target.width, target.height);
        {
            const stats::Scope filter("GTAO filter", stats::kGpu);
            m_composite.use();
            glUniformMatrix4fv(m_composite.uniform("projection"), 1, GL_FALSE, p.data());
            glUniform1i(m_composite.uniform("evaluation_scale"), scale);
            glUniform1f(m_composite.uniform("strength"), std::clamp(options.strength, 0.0f, 3.0f));
            if (apply_to_colour) {
                glEnable(GL_BLEND);
                // Legacy prelit paths do not expose separate indirect illumination.
                glBlendEquation(GL_FUNC_ADD);
                glBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE);
            }
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
    }
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glDepthMask(GL_TRUE);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(world::gl::GL_TEXTURE_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
    glViewport(0, 0, target.width, target.height);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void Gtao::release() {
    m_sampled_depth = 0;
    m_depth.release();
    if (m_ao_texture) {
        glDeleteTextures(1, &m_ao_texture);
    }
    if (m_ao_buffer) {
        glDeleteBuffers(1, &m_ao_buffer);
    }
    m_ao_texture = m_ao_buffer = 0;
    m_ao_width = m_ao_height = 0;
    m_visibility.release();
    m_evaluate.release();
    m_evaluate_performance.release();
    m_composite.release();
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
    }
    m_vao = 0;
    m_failed = false;
}
}  // namespace openrac::renderer
