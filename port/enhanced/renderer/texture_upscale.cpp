// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Bounded Vulkan colour-reconstruction cache for world textures.
#include "enhanced/renderer/texture_upscale.h"

#include <algorithm>

#include "common/log.h"
#include "enhanced_texture_shaders.h"
#include "renderer/gl.h"
#include "renderer/texture_pool.h"
#if defined(OPENRAC_VULKAN) && OPENRAC_VULKAN
#include "renderer/vulkan/vkgl.h"
#endif
namespace openrac::enhanced {
using namespace openrac::gl;

void TextureUpscaler::release(Entry& entry) {
    for (auto& texture : entry.textures) {
        if (texture) {
            glDeleteTextures(1, &texture);
            m_cache_bytes -= entry.bytes;
            texture = 0;
        }
    }
}

bool TextureUpscaler::reserve(std::size_t bytes, std::uint64_t frame) {
    if (bytes > kCacheBudget) {
        return false;
    }

    // Keep recently used colours to avoid rebuilding an oversized working set each frame.
    while (m_cache_bytes + bytes > kCacheBudget) {
        Entry* oldest = nullptr;

        for (auto& [source, entry] : m_cache) {
            const bool cached =
                std::any_of(entry.textures.begin(), entry.textures.end(), [](unsigned id) {
                    return id != 0;
                });

            if (cached && frame - entry.last_used >= renderer::TexturePool::kKeepFrames
                && (!oldest || entry.last_used < oldest->last_used)) {
                oldest = &entry;
            }
        }

        if (!oldest) {
            return false;
        }

        release(*oldest);
    }

    return true;
}

unsigned TextureUpscaler::prepare(
    unsigned source, int width, int height, unsigned wrap, std::uint64_t frame
) {
    if (!enhanced_rendering_available() || width <= 1 || height <= 1) {
        return 0;
    }

    auto& entry = m_cache[source];
    entry.last_used = frame;
    auto& result = entry.textures[wrap];

    if (result) {
        return result;
    }

    const auto bytes = byte_size(width, height);

    if (!reserve(bytes, frame)) {
        return 0;
    }

    std::string error;
    result = create(source, width, height, wrap, error);

    if (result) {
        entry.bytes = bytes;
        m_cache_bytes += bytes;
        ++m_cache_builds;
    } else {
        static log::Capped told(1);

        if (told()) {
            log::warn("{}; using Normal filtering", error);
        }
    }

    return result;
}

void TextureUpscaler::forget(unsigned source) {
    auto found = m_cache.find(source);

    if (found != m_cache.end()) {
        release(found->second);
        m_cache.erase(found);
    }
}

unsigned TextureUpscaler::create(
    unsigned source, int width, int height, unsigned wrap, std::string& error
) {
#if defined(OPENRAC_VULKAN) && OPENRAC_VULKAN
    if (!enhanced_rendering_available()) {
        return 0;
    }

    if (!m_shader.id()) {
        if (!m_shader.build_compute("texture upscale", shaders::texture_upscale_comp, error)) {
            return 0;
        }

        glGenBuffers(1, &m_buffer);
    }

    unsigned result = 0;
    glGenTextures(1, &result);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, result);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA8, width * 2, height * 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (wrap & 1) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (wrap & 2) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    m_shader.use();
    glUniform1i(m_shader.uniform("tex_T0"), 0);
    glUniform1i(m_shader.uniform("wrap_mode"), static_cast<int>(wrap));
    glBindTexture(GL_TEXTURE_2D, source);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        static_cast<GLsizeiptr>(byte_size(width, height)),
        nullptr,
        GL_DYNAMIC_DRAW
    );
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_buffer);
    glDispatchCompute(
        static_cast<unsigned>((width * 2 + 7) / 8), static_cast<unsigned>((height * 2 + 7) / 8), 1
    );
    const bool ok = vkgl::copy_buffer_to_texture(m_buffer, result, width * 2, height * 2);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

    if (!ok) {
        glDeleteTextures(1, &result);
        error = "texture upscale output failed";
        return 0;
    }

    return result;
#else
    (void)source;
    (void)width;
    (void)height;
    (void)wrap;
    error = "edge-aware texture upscaling requires Vulkan";
    return 0;
#endif
}

void TextureUpscaler::release() {
    for (auto& [source, entry] : m_cache) {
        release(entry);
    }

    m_cache.clear();
    m_shader.release();

    if (m_buffer) {
        glDeleteBuffers(1, &m_buffer);
    }

    m_buffer = 0;
}
}  // namespace openrac::enhanced
