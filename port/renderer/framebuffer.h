// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// An off-screen render target: an RGBA8 colour texture and a depth-stencil
// buffer. The renderer draws a frame into one (OpenGOAL renders the game at
// its own resolution and scales it to the window), and the headless checks
// read their pixels back from one.
//
// With more than one sample a pixel (anti-aliasing) the target drawn into is
// a pair of multisampled buffers, which cannot be sampled, read or scaled:
// the colour texture then belongs to a second framebuffer the samples are
// resolved into (resolved()), as OpenGOAL does for its own multisampled frame. Where the GPU resolves
// the samples itself (resolves_itself()), the second framebuffer holds the texture they go to.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace openrac::renderer {

// A framebuffer's colour (one sample a pixel) as RGBA bytes, top row first.
std::vector<std::uint8_t> read_rgba(unsigned framebuffer, int width, int height);

class FrameBuffer {
public:
    FrameBuffer() = default;
    ~FrameBuffer();
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    // `samples`: 1 for a plain target; more is held to what the driver offers (max_samples()).
    bool create(int width, int height, std::string& error, int samples = 1);
    void release();

    // The framebuffer to draw into.
    unsigned id() const { return m_framebuffer; }

    // What was drawn, as a framebuffer with one sample a pixel, for reading it, sampling it or
    // copying it at another size: id() itself, or the multisampled target resolved now.
    unsigned resolved() const;

    // The resolved colour (after resolved(), when the target is multisampled).
    unsigned colour_texture() const { return m_colour; }

    int samples() const { return m_samples; }

    // The framebuffer resolved() fills, without filling it; 0 for a target of one sample a pixel.
    unsigned resolve_target() const { return m_resolve; }

    // The samples are resolved into the colour texture by the GPU itself, as they are written out
    // (GL_EXT_multisampled_render_to_texture): resolve_target() can be read at any time, with
    // nothing to copy first.
    bool resolves_itself() const { return m_implicit; }

    // The most samples a pixel the driver gives a target (1 when it has no multisampling).
    static int max_samples();

    int width() const { return m_width; }

    int height() const { return m_height; }

    // The colour buffer as RGBA bytes, top row first.
    std::vector<std::uint8_t> read_rgba() const;

    // Copy to another framebuffer (0: the window), scaled to fill
    // dst_width x dst_height with linear filtering.
    void blit_to(unsigned target, int dst_width, int dst_height) const;

private:
    unsigned m_framebuffer = 0;
    unsigned m_colour = 0;
    unsigned m_depth = 0;
    unsigned m_resolve = 0;       // the framebuffer holding m_colour, when multisampled
    unsigned m_multisample = 0;   // the multisampled colour buffer
    int m_samples = 1;
    bool m_implicit = false;      // resolves_itself()
    int m_width = 0;
    int m_height = 0;
};

}  // namespace openrac::renderer
