// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Single-sample intermediate colour targets; optional world depth is borrowed, never freed here.
#pragma once

#include <string>

namespace openrac::enhanced::fsr3_detail {
// Owns one colour image and framebuffer on the render thread.
class FrameTarget {
public:
    FrameTarget() = default;
    // Releases colour and framebuffer while the renderer's context remains current.
    ~FrameTarget();
    FrameTarget(const FrameTarget&) = delete;
    FrameTarget& operator=(const FrameTarget&) = delete;
    // Replaces the target; `format` is a colour texture format and `depth` is a borrowed attachment.
    [[nodiscard]] bool create(
        int width, int height, unsigned format, unsigned depth, std::string& error
    );
    // Releases owned objects without deleting a borrowed depth texture.
    void release();

    // Returns the colour texture name.
    unsigned texture() const { return m_texture; }

    // Returns the framebuffer name.
    unsigned framebuffer() const { return m_framebuffer; }

private:
    unsigned m_texture = 0;      // Owned single-level colour texture.
    unsigned m_framebuffer = 0;  // Owned framebuffer, with an optional borrowed depth attachment.
};
}  // namespace openrac::enhanced::fsr3_detail
