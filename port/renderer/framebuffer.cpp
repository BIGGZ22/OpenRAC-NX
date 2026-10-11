// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/framebuffer.h"

#include <algorithm>
#include <format>

#include "renderer/gl.h"

namespace openrac::renderer {

using namespace openrac::gl;

FrameBuffer::~FrameBuffer() {
    // release() frees the GL objects while the context exists.
}

bool FrameBuffer::create(int width, int height, std::string& error) {
    release();
    m_width = width;
    m_height = height;
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
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colour, 0);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth
    );
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        error = std::format("framebuffer {}x{} incomplete ({:#x})", width, height, status);
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
    if (m_colour != 0) {
        glDeleteTextures(1, &m_colour);
        m_colour = 0;
    }
    if (m_depth != 0) {
        glDeleteRenderbuffers(1, &m_depth);
        m_depth = 0;
    }
}

std::vector<std::uint8_t> FrameBuffer::read_rgba() const {
    const auto row = static_cast<std::size_t>(m_width) * 4;
    std::vector<std::uint8_t> pixels(row * static_cast<std::size_t>(m_height));
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    // GL's rows run bottom to top.
    for (int y = 0; y < m_height / 2; ++y) {
        auto top = pixels.begin() + static_cast<std::ptrdiff_t>(static_cast<std::size_t>(y) * row);
        auto bottom =
            pixels.begin()
            + static_cast<std::ptrdiff_t>(static_cast<std::size_t>(m_height - 1 - y) * row);
        std::swap_ranges(top, top + static_cast<std::ptrdiff_t>(row), bottom);
    }
    return pixels;
}

void FrameBuffer::blit_to(unsigned target, int dst_width, int dst_height) const {
    // A blit is clipped by the scissor and the colour mask like a draw: neither applies to a copy
    // (the renderers set both again for their own draws).
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);
    glBlitFramebuffer(
        0, 0, m_width, m_height, 0, 0, dst_width, dst_height, GL_COLOR_BUFFER_BIT, GL_LINEAR
    );
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

}  // namespace openrac::renderer
