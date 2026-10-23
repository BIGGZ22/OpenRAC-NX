// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <functional>

#include "renderer/renderer.h"
#include "renderer/shader.h"

namespace openrac::viewer {
// One RGBA8 texel per 2x2 screen pixels: log-encoded irradiance RGB and
// linear cascade visibility A. The 8-byte guide stores surface identity and
// Ratchet shadow visibility in its remaining bits.
class EarlyLighting {
public:
    static bool supported();
    bool render(
        unsigned depth,
        const renderer::FrameInput& input,
        const renderer::RenderTarget& target,
        const std::function<void(const renderer::Shader&)>& bind_lighting,
        std::string& error
    );
    void bind(
        const renderer::Shader& shader,
        const renderer::FrameInput& input,
        const renderer::RenderState& state
    ) const;
    bool matches(const renderer::FrameInput& input, const renderer::RenderState& state) const;
    void release();

private:
    renderer::Shader m_compute;
    std::array<unsigned, 2> m_buffers{}, m_textures{};
    int m_width = 0, m_height = 0;
    unsigned m_target = 0;
    renderer::Mat4 m_clip{};
    std::uint64_t m_frame = 0;
    bool m_ready = false;
    bool m_failed = false;
};
}  // namespace openrac::viewer
