// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A picture made on the CPU, drawn over a framebuffer: its RGBA8 pixels are uploaded to a texture
// of its own at each draw and blitted, with nearest filtering, into a rectangle of the target. The
// debug menu's level list (game/common/debug_menu.cpp) is drawn this way: the menu lays out its
// picture, this draws it, so the menu's logic holds no GL.
//
// It keeps one texture and one read framebuffer, made at the first draw and freed by release(),
// while the context is current. The destructor frees nothing, since the context may be gone by
// then, as with renderer::FrameBuffer.

#pragma once

#include <cstdint>
#include <span>

namespace openrac::renderer::ui {

// An RGBA8 picture, `width` x `height` pixels, its first row the top. The bytes are the caller's.
struct PictureView {
    std::span<const std::uint8_t> rgba;  // width * height * 4 bytes, R G B A.
    int width = 0;                       // Pixels.
    int height = 0;                      // Pixels.
};

// Where a picture lands in the target: its bottom left corner and size, in the target's pixels,
// rows counted from the bottom as GL counts them.
struct PictureRect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

// Draws CPU-made pictures over a framebuffer; one per user, which owns its GL objects.
class PictureBlit {
public:
    PictureBlit() = default;
    PictureBlit(const PictureBlit&) = delete;
    PictureBlit& operator=(const PictureBlit&) = delete;

    // Uploads `picture` and blits it into `to` of `framebuffer` (0: the window's), scaled with
    // nearest filtering and its top row at the rectangle's top. The context is current. Leaves the
    // scissor test off and the read and draw framebuffers at 0.
    void draw(unsigned framebuffer, const PictureView& picture, const PictureRect& to);

    // Frees the texture and the read framebuffer, if a draw made them (the context is current).
    void release();

private:
    unsigned m_texture = 0;      // The picture's texture; 0 until the first draw.
    unsigned m_framebuffer = 0;  // The read framebuffer the texture is attached to.
};

}  // namespace openrac::renderer::ui
