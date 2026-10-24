// SPDX-License-Identifier: GPL-3.0-or-later
#include "enhanced/viewer/probe_selection.h"

#include <vector>

#include "common/stats.h"
#include "renderer/gl.h"
#include "renderer/world/gl_world.h"
#include "viewer_shaders.h"

namespace openrac::scene {
using namespace openrac::gl;
namespace wgl = renderer::world::gl;

namespace {

void block(const renderer::Shader& shader) {
    const auto at = glGetUniformBlockIndex(shader.id(), "ProbeRegions");
    if (at != wgl::GL_INVALID_INDEX) {
        glUniformBlockBinding(shader.id(), at, 7);
    }
}
}  // namespace

bool ProbeSelection::supported() {
    if (!enhanced_rendering_available()) {
        return false;
    }
    static const bool& off =
        stats::probe("OPENRAC_PROBE_COMPUTE_OFF", "Use the CPU/frustum probe selector");
    if (off || !glDispatchCompute || !glMemoryBarrier) {
        return false;
    }
    GLint bindings = 0;
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &bindings);
    return bindings >= 9;
}

void ProbeSelection::ensure_buffers() const {
    if (m_buffers[0]) {
        return;
    }
    glGenBuffers(2, m_buffers.data());
    std::array<float, capacity * 3 * 4> zeros{};
    glBindBuffer(wgl::GL_UNIFORM_BUFFER, m_buffers[0]);
    glBufferData(wgl::GL_UNIFORM_BUFFER, sizeof(zeros), zeros.data(), GL_STATIC_DRAW);
    glBindBuffer(wgl::GL_TEXTURE_BUFFER, m_buffers[1]);
    glBufferData(wgl::GL_TEXTURE_BUFFER, sizeof(unsigned), zeros.data(), GL_DYNAMIC_DRAW);
    glGenTextures(1, &m_mask_texture);
    glActiveTexture(GL_TEXTURE0 + 15);
    glBindTexture(wgl::GL_TEXTURE_BUFFER, m_mask_texture);
    glTexBuffer(wgl::GL_TEXTURE_BUFFER, wgl::GL_R32UI, m_buffers[1]);
    glActiveTexture(GL_TEXTURE0);
}

bool ProbeSelection::prepare(
    std::span<const lighting::ProbeGrid> grids,
    std::span<const unsigned> offsets,
    std::string& error
) {
    release();
    ensure_buffers();
    if (!supported() || grids.empty() || grids.size() > capacity
        || offsets.size() != grids.size()) {
        return false;
    }
    if (!m_cull.build_compute("tiled probe-volume culling", shaders::probe_selection_comp, error)) {
        return false;
    }
    m_count = static_cast<unsigned>(grids.size());
    std::array<std::array<float, 4>, capacity * 3> metadata{};
    for (unsigned i = 0; i < m_count; ++i) {
        for (unsigned a = 0; a < 3; ++a) {
            metadata[i * 3][a] = grids[i].origin[a];
            metadata[i * 3 + 1][a] = grids[i].spacing[a];
            metadata[i * 3 + 2][a] = float(grids[i].dimensions[a]);
        }
        metadata[i * 3][3] = float(offsets[i]);
    }
    glBindBuffer(wgl::GL_UNIFORM_BUFFER, m_buffers[0]);
    glBufferData(wgl::GL_UNIFORM_BUFFER, sizeof(metadata), metadata.data(), GL_STATIC_DRAW);
    block(m_cull);
    return true;
}

void ProbeSelection::dispatch(
    unsigned depth, const renderer::Camera& camera, int width, int height
) {
    if (!available() || !depth || width <= 0 || height <= 0) {
        return;
    }
    const unsigned words = (m_count + 31) / 32;
    const auto tiles_x = (width + 15) / 16, tiles_y = (height + 15) / 16;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, m_buffers[1]);
    if (width != m_width || height != m_height) {
        std::vector<unsigned> zeros(std::size_t(tiles_x) * tiles_y * words, 0);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffers[1]);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER, zeros.size() * sizeof(unsigned), zeros.data(), GL_DYNAMIC_DRAW
        );
    }
    glBindBufferBase(wgl::GL_UNIFORM_BUFFER, 7, m_buffers[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depth);
    m_clip = renderer::multiply(camera.projection, camera.view);
    glMemoryBarrier(
        GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT
    );  // texture-fetch readers
    m_cull.use();
    glUniform1i(m_cull.uniform("region_count"), static_cast<int>(m_count));
    glUniform1i(m_cull.uniform("scene_depth"), 0);
    glUniformMatrix4fv(m_cull.uniform("world_to_clip"), 1, GL_FALSE, m_clip.data());
    glDispatchCompute(static_cast<unsigned>(tiles_x), static_cast<unsigned>(tiles_y), 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    m_started = true;
    m_width = width;
    m_height = height;
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ProbeSelection::bind(
    const renderer::Shader& shader, const renderer::Camera* camera, bool depth_matched
) const {
    ensure_buffers();
    GLint viewport[4]{};
    glGetIntegerv(GL_VIEWPORT, viewport);
    static const bool& reference = stats::probe(
        "OPENRAC_PROBE_TILED_REFERENCE", "Evaluate all resident probe volumes per fragment"
    );
    const bool same_view =
        !camera || renderer::multiply(camera->projection, camera->view) == m_clip;
    const bool tiled = depth_matched && !reference && same_view && viewport[0] == 0
                       && viewport[1] == 0 && viewport[2] == m_width && viewport[3] == m_height;
    // Blended receivers can lie outside opaque depth bounds, even in the same view.
    // They and secondary cameras use all resident regions, never opaque-only masks.
    glUniform1i(shader.uniform("probe_gpu_selection"), active() ? (tiled ? 1 : 2) : 0);
    glUniform1i(shader.uniform("probe_region_count"), static_cast<int>(m_count));
    glUniform2f(shader.uniform("probe_tile_extent"), float(m_width), float(m_height));
    glUniform1i(shader.uniform("probe_region_masks"), 15);
    block(shader);
    glBindBufferBase(wgl::GL_UNIFORM_BUFFER, 7, m_buffers[0]);
    glActiveTexture(GL_TEXTURE0 + 15);
    glBindTexture(wgl::GL_TEXTURE_BUFFER, m_mask_texture);
    glActiveTexture(GL_TEXTURE0);
}

void ProbeSelection::release() {
    m_cull.release();
    if (m_buffers[0]) {
        glDeleteBuffers(2, m_buffers.data());
    }
    if (m_mask_texture) {
        glDeleteTextures(1, &m_mask_texture);
    }
    m_buffers = {};
    m_mask_texture = 0;
    m_width = m_height = 0;
    m_count = 0;
    m_started = false;
}
}  // namespace openrac::scene
