// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/framebuffer.h"

#include <algorithm>
#include <format>

#include "renderer/gl.h"

namespace openrac::renderer {

using namespace openrac::gl;

namespace {

// A driver that lists GL_EXT_multisampled_render_to_texture but would not complete such a target:
// multisampled buffers from then on.
bool g_implicit_refused = false;

}  // namespace

FrameBuffer::~FrameBuffer() {
    // release() frees the GL objects while the context exists.
}

int FrameBuffer::max_samples() {
    static int most = 0;
    if (most == 0) {
        GLint value = 1;
        glGetIntegerv(GL_MAX_SAMPLES, &value);
        most = std::max(1, static_cast<int>(value));
    }
    return most;
}

bool FrameBuffer::create(int width, int height, std::string& error, int samples) {
    release();
    m_width = width;
    m_height = height;
    m_samples = std::clamp(samples, 1, max_samples());
    glGenTextures(1, &m_colour);
    glBindTexture(GL_TEXTURE_2D, m_colour);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        static_cast<GLint>(GL_RGBA8),
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(GL_LINEAR));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(GL_LINEAR));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(GL_CLAMP_TO_EDGE));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(GL_CLAMP_TO_EDGE));

    glGenRenderbuffers(1, &m_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    // On a tiled GPU with GL_EXT_multisampled_render_to_texture (a phone's) the samples never
    // leave the GPU's tile memory: they are resolved into the colour texture as each tile is
    // written out, and the depth's samples are not written at all. A multisampled buffer instead
    // writes every sample to memory and reads it back for the resolve, four times a plain frame's
    // traffic at 4x, each frame. A second framebuffer holds the same texture with one sample, for
    // reading and copying what was drawn.
    m_implicit = m_samples > 1 && !g_implicit_refused && glFramebufferTexture2DMultisampleEXT != nullptr
                 && glRenderbufferStorageMultisampleEXT != nullptr;
    if (m_implicit) {
        glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, m_samples, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colour, 0,
                                             m_samples);
    } else if (m_samples > 1) {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, GL_DEPTH24_STENCIL8, width, height);
        glGenRenderbuffers(1, &m_multisample);
        glBindRenderbuffer(GL_RENDERBUFFER, m_multisample);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, GL_RGBA8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_multisample);
    } else {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colour, 0);
    }
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth
    );
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status == GL_FRAMEBUFFER_COMPLETE && m_samples > 1) {
        glGenFramebuffers(1, &m_resolve);
        glBindFramebuffer(GL_FRAMEBUFFER, m_resolve);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colour, 0);
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (status != GL_FRAMEBUFFER_COMPLETE && m_implicit) {
        g_implicit_refused = true;
        return create(width, height, error, samples);
    }
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        error = std::format("framebuffer {}x{} with {} sample(s) incomplete ({:#x})", width, height, m_samples,
                            status);
        release();
        return false;
    }
    return true;
}

void FrameBuffer::release() {
    if (m_framebuffer != 0) {
        glDeleteFramebuffers(1, &m_framebuffer);
        m_framebuffer = 0;
    }
    if (m_resolve != 0) {
        glDeleteFramebuffers(1, &m_resolve);
        m_resolve = 0;
    }
    if (m_colour != 0) {
        glDeleteTextures(1, &m_colour);
        m_colour = 0;
    }
    if (m_depth != 0) {
        glDeleteRenderbuffers(1, &m_depth);
        m_depth = 0;
    }
    if (m_multisample != 0) {
        glDeleteRenderbuffers(1, &m_multisample);
        m_multisample = 0;
    }
    m_samples = 1;
    m_implicit = false;
    m_width = 0;
    m_height = 0;
}

unsigned FrameBuffer::resolved() const {
    if (m_resolve == 0) {
        return m_framebuffer;
    }
    if (m_implicit) {
        return m_resolve;  // the texture holds the resolved colour already
    }
    // As blit_to: neither the scissor nor the colour mask applies to a copy.
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_resolve);
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    return m_resolve;
}

std::vector<std::uint8_t> read_rgba(unsigned framebuffer, int width, int height) {
    const auto row = static_cast<std::size_t>(width) * 4;
    std::vector<std::uint8_t> pixels(row * static_cast<std::size_t>(height));
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    // GL's rows run bottom to top.
    for (int y = 0; y < height / 2; ++y) {
        auto top = pixels.begin() + static_cast<std::ptrdiff_t>(static_cast<std::size_t>(y) * row);
        auto bottom =
            pixels.begin()
            + static_cast<std::ptrdiff_t>(static_cast<std::size_t>(height - 1 - y) * row);
        std::swap_ranges(top, top + static_cast<std::ptrdiff_t>(row), bottom);
    }
    return pixels;
}

std::vector<std::uint8_t> FrameBuffer::read_rgba() const {
    return renderer::read_rgba(resolved(), m_width, m_height);
}

void FrameBuffer::blit_to(unsigned target, int dst_width, int dst_height) const {
    // A blit is clipped by the scissor and the colour mask like a draw: neither applies to a copy
    // (the renderers set both again for their own draws).
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    // A multisampled target copies to one of another size only through its resolved colour.
    const unsigned source = m_implicit || (m_samples > 1 && (dst_width != m_width || dst_height != m_height))
                                ? resolved()
                                : m_framebuffer;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, source);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);
    const bool same_size = dst_width == m_width && dst_height == m_height;
    glBlitFramebuffer(
        0, 0, m_width, m_height, 0, 0, dst_width, dst_height, GL_COLOR_BUFFER_BIT,
        same_size ? GL_NEAREST : GL_LINEAR
    );
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

}  // namespace openrac::renderer
