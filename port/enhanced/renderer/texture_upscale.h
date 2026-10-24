// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Own cached colour textures independently of the original texture pool.
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "renderer/shader.h"
#include "renderer/texture_filter.h"
#include "renderer/texture_pool.h"

namespace openrac::enhanced {
// Installs the optional colour provider once, after a supported graphics context is current.
void install_texture_upscaler(renderer::TexturePool& textures);

// Immutable 2x RGB textures. Original alpha, mipmaps and non-colour uses stay in the pool.
class TextureUpscaler final : public renderer::UpscaledTextureColours {
public:
    unsigned prepare(
        unsigned source, int width, int height, unsigned wrap, std::uint64_t frame
    ) override;
    void forget(unsigned source) override;
    void release() override;

    std::size_t cache_bytes() const { return m_cache_bytes; }

    std::uint64_t cache_builds() const { return m_cache_builds; }

    static constexpr std::size_t kCacheBudget = 64 * 1024 * 1024;

private:
    struct Entry {
        std::array<unsigned, 4> textures{};
        std::size_t bytes = 0;
        std::uint64_t last_used = 0;
    };

    unsigned create(unsigned source, int width, int height, unsigned wrap, std::string& error);
    bool reserve(std::size_t bytes, std::uint64_t frame);
    void release(Entry& entry);

    static constexpr std::size_t byte_size(int width, int height) {
        return static_cast<std::size_t>(width) * height * 16;  // 2x width/height, RGBA8
    }

    renderer::Shader m_shader;
    unsigned m_buffer = 0;
    std::unordered_map<unsigned, Entry> m_cache;
    std::size_t m_cache_bytes = 0;
    std::uint64_t m_cache_builds = 0;
};
}  // namespace openrac::enhanced
