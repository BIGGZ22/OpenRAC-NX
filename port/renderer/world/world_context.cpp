// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/world/world_context.h"

#include <algorithm>
#include <array>

#include "renderer/world/gl_world.h"

namespace openrac::renderer::world {

using namespace openrac::renderer::world::gl;

namespace {

// std140 WorldView (world_common.glsl).
struct ViewBlock {
    Mat4 view;
    Mat4 projection;
    Mat4 view_projection;
    std::array<float, 4> eye;
    std::array<float, 4> fog_colour;
    std::array<float, 4> fog_params;
    std::array<float, 4> screen;
    std::array<float, 4> tans;
};

static_assert(sizeof(ViewBlock) == 272);

// std140 WorldLights (world_lights.glsl).
struct LightsBlock {
    std::array<std::array<float, 4>, kPointLightSlots> position;
    std::array<std::array<float, 4>, kPointLightSlots> colour;
};

static_assert(sizeof(LightsBlock) == 256);

}  // namespace

WorldContext::WorldContext() = default;

WorldContext::~WorldContext() = default;

bool WorldContext::init(std::string& error) {
    if (m_initialised) {
        return true;
    }
    if (!gl::loaded()) {
        error =
            "world renderers: world::gl::load() has not been called (renderer/world/gl_world.h)";
        return false;
    }
    glGenVertexArrays(1, &m_empty_vao);
    m_initialised = true;
    return true;
}

void WorldContext::release() {
    m_view_block.release();
    m_lights_block.release();
    m_samplers.release();
    if (m_scratch != 0) {
        glDeleteTextures(1, &m_scratch);
        m_scratch = 0;
    }
    if (m_scratch_framebuffer != 0) {
        glDeleteFramebuffers(1, &m_scratch_framebuffer);
        m_scratch_framebuffer = 0;
    }
    if (m_empty_vao != 0) {
        glDeleteVertexArrays(1, &m_empty_vao);
        m_empty_vao = 0;
    }
    m_uploaded.clear();
    m_scratch_width = 0;
    m_scratch_height = 0;
    m_initialised = false;
}

void WorldContext::prepare(const FrameInput& input, const RenderTarget& target) {
    (void)target;
    m_gl_camera = input.camera;
    m_camera = WorldCamera::from(input.camera);
    const WorldFrame& f = frame();
    ViewBlock v{};
    v.view = input.camera.view;
    v.projection = input.camera.projection;
    v.view_projection = multiply(input.camera.projection, input.camera.view);
    v.eye = {m_camera.eye[0], m_camera.eye[1], m_camera.eye[2], 0.0f};
    v.fog_colour =
        {f.fog.colour[0] / 255.0f,
         f.fog.colour[1] / 255.0f,
         f.fog.colour[2] / 255.0f,
         f.fog.enabled ? 1.0f : 0.0f};
    v.fog_params = fog_params(f.fog);
    v.screen = {
        m_profile.frame_width * 0.5f, m_profile.frame_height * 0.5f, kNear, f.lod_tint ? 1.0f : 0.0f
    };
    v.tans = {m_camera.tan_x, m_camera.tan_y, m_profile.frame_y_ratio, 0.0f};
    m_view_block.upload(GL_UNIFORM_BUFFER, &v, sizeof v, true);
    glBindBufferBase(GL_UNIFORM_BUFFER, kWorldViewBinding, m_view_block.id());

    LightsBlock l{};
    for (std::size_t i = 0; i < kPointLightSlots; ++i) {
        const PointLight& p = f.lights[i];
        l.position[i] = {p.position[0], p.position[1], p.position[2], p.radius};
        l.colour[i] = {p.colour[0], p.colour[1], p.colour[2], p.intensity};
    }
    m_lights_block.upload(GL_UNIFORM_BUFFER, &l, sizeof l, true);
    glBindBufferBase(GL_UNIFORM_BUFFER, kWorldLightsBinding, m_lights_block.id());
}

void WorldContext::bind_texture(
    TexturePool& pool, const WorldTexture& texture, const SamplerState& sampler, int unit
) {
    const TextureHandle handle = texture.texture != 0 ? texture.texture : pool.placeholder();
    const unsigned id = pool.gl_texture(handle);
    glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(unit));
    glBindTexture(GL_TEXTURE_2D, id);
    const auto known = std::find_if(m_uploaded.begin(), m_uploaded.end(), [&](const Uploaded& u) {
        return u.handle == handle;
    });
    const bool fresh = known == m_uploaded.end() || known->gl != id;
    if (fresh) {
        // The game's own mip chain (levels 1..MXL; the pool holds level 0).
        // A level of the wrong size ends the chain there.
        const Rgba8Image& base = pool.image(handle);
        int levels = 0;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        for (std::size_t i = 0; handle == texture.texture && i < texture.mip_levels.size(); ++i) {
            const Rgba8Image& m = texture.mip_levels[i];
            const int level = static_cast<int>(i) + 1;
            if (m.width != std::max(1, base.width >> level)
                || m.height != std::max(1, base.height >> level)
                || m.pixels.size() != static_cast<std::size_t>(m.width * m.height * 4)) {
                break;
            }
            glTexImage2D(
                GL_TEXTURE_2D,
                level,
                static_cast<GLint>(GL_RGBA8),
                m.width,
                m.height,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                m.pixels.data()
            );
            levels = level;
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels);
        if (known == m_uploaded.end()) {
            m_uploaded.push_back({id, handle});
        } else {
            known->gl = id;
        }
    }
    SamplerState s = sampler;
    if (texture.mip_levels.empty()) {
        s.mipmapped = false;
    }
    glBindSampler(static_cast<GLuint>(unit), m_samplers.get(s));
}

void WorldContext::bind_gl_texture(unsigned gl_texture, const SamplerState& sampler, int unit) {
    glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(unit));
    glBindTexture(GL_TEXTURE_2D, gl_texture);
    glBindSampler(static_cast<GLuint>(unit), m_samplers.get(sampler));
}

unsigned WorldContext::copy_target(const RenderTarget& target) {
    if (m_scratch == 0 || m_scratch_width != target.width || m_scratch_height != target.height) {
        if (m_scratch == 0) {
            glGenTextures(1, &m_scratch);
        }
        glBindTexture(GL_TEXTURE_2D, m_scratch);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            static_cast<GLint>(GL_RGBA8),
            target.width,
            target.height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        if (m_scratch_framebuffer == 0) {
            glGenFramebuffers(1, &m_scratch_framebuffer);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, m_scratch_framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_scratch, 0);
        m_scratch_width = target.width;
        m_scratch_height = target.height;
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, target.framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_scratch_framebuffer);
    glBlitFramebuffer(
        0,
        0,
        target.width,
        target.height,
        0,
        0,
        target.width,
        target.height,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
    );
    glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
    return m_scratch;
}

void WorldContext::draw_fullscreen() {
    glBindVertexArray(m_empty_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

}  // namespace openrac::renderer::world
