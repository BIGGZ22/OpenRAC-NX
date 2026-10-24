// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The picture blit: the upload, the blit and the release of picture_blit.h, in the GL calls the
// debug menu made itself before restructure package P2-06 moved them here.

#include "renderer/ui/picture_blit.h"

#include "renderer/gl.h"

namespace openrac::renderer::ui {

using namespace openrac::gl;

void PictureBlit::draw(unsigned framebuffer, const PictureView& picture, const PictureRect& to) {
    if (m_texture == 0) {
        // The first draw: the texture and the framebuffer that reads it, kept for the next ones.
        glGenTextures(1, &m_texture);
        glGenFramebuffers(1, &m_framebuffer);
    }
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        static_cast<GLint>(GL_RGBA8),
        picture.width,
        picture.height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        picture.rgba.data()
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(GL_NEAREST));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(GL_NEAREST));
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
    glDisable(GL_SCISSOR_TEST);
    // The picture's first row is its top; GL's framebuffer counts rows from the bottom.
    glBlitFramebuffer(
        0,
        0,
        picture.width,
        picture.height,
        to.x,
        to.y + to.height,
        to.x + to.width,
        to.y,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
    );
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void PictureBlit::release() {
    // Nothing to free before the first draw.
    if (m_texture == 0) {
        return;
    }
    glDeleteTextures(1, &m_texture);
    glDeleteFramebuffers(1, &m_framebuffer);
    m_texture = 0;
    m_framebuffer = 0;
}

}  // namespace openrac::renderer::ui
