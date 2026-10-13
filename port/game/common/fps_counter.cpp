// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "fps_counter.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <format>
#include <string>
#include <vector>

#include "renderer/gl.h"

namespace openrac::frontend::fps_counter {

namespace {

using namespace openrac::gl;

// 5 x 7 glyphs, 7 rows of 5 columns each ('#' lit): what the counter's text needs.
struct Glyph {
    char c;
    const char* rows;
};

constexpr Glyph kFont[] = {
    {'0', " ### #   ##  ### # ###  ##   # ### "}, {'1', "  #   ##    #    #    #    #   ### "},
    {'2', " ### #   #    #   #   #   #   #####"}, {'3', "#####   #   #     #     ##   # ### "},
    {'4', "   #   ##  # # #  # #####   #    # "}, {'5', "######    ####     #    ##   # ### "},
    {'6', "  ##  #   #    #### #   ##   # ### "}, {'7', "#####    #   #   #   #    #    #   "},
    {'8', " ### #   ##   # ### #   ##   # ### "}, {'9', " ### #   ##   # ####    #   #  ##  "},
    {'.', "                          ##   ##  "}, {'F', "######    #    #### #    #    #    "},
    {'P', "#### #   ##   ##### #    #    #    "}, {'S', " #####    #     ###     #    ##### "},
    {'M', "#   ### ### # ##   ##   ##   ##   #"},
};

const char* glyph(char c) {
    for (const Glyph& g : kFont) {
        if (g.c == c) {
            return g.rows;
        }
    }
    return nullptr;
}

constexpr int kCell = 6;    // a glyph and the column after it
constexpr int kMargin = 2;  // around the text, inside its box
constexpr int kRows = 7;

struct State {
    // The frames since the text was last made, and when that was.
    std::chrono::steady_clock::time_point since{};
    int frames = 0;
    double cost = 0.0;
    std::string text;
    std::string drawn;  // the text the texture holds
    int width = 0;
    unsigned texture = 0;
    unsigned framebuffer = 0;
};
State s;

// The text as a picture: light glyphs on a dark box, top row first.
std::vector<std::uint8_t> picture(const std::string& text, int& width, int& height) {
    width = static_cast<int>(text.size()) * kCell - 1 + 2 * kMargin;
    height = kRows + 2 * kMargin;
    std::vector<std::uint8_t> rgba(static_cast<std::size_t>(width) * height * 4);
    for (std::size_t i = 0; i < rgba.size(); i += 4) {
        const std::uint32_t dark = 0xFF101010u;
        std::memcpy(&rgba[i], &dark, 4);
    }
    int x = kMargin;
    for (char c : text) {
        if (const char* rows = glyph(c)) {
            for (int r = 0; r < kRows; ++r) {
                for (int col = 0; col < 5; ++col) {
                    if (rows[r * 5 + col] == '#') {
                        const std::uint32_t light = 0xFF40FFF0u;  // yellow-green, as A B G R bytes
                        std::memcpy(&rgba[(static_cast<std::size_t>(kMargin + r) * width + x + col) * 4], &light, 4);
                    }
                }
            }
        }
        x += kCell;
    }
    return rgba;
}

}  // namespace

void frame_drawn(std::chrono::steady_clock::time_point now, double cost) {
    using namespace std::chrono;
    if (s.since.time_since_epoch().count() == 0) {
        s.since = now;
    }
    ++s.frames;
    s.cost += cost;
    // Four times a second: fast enough to follow, slow enough to read.
    const double seconds = duration<double>(now - s.since).count();
    if (seconds >= 0.25) {
        s.text = std::format("{:.0f} FPS {:.1f} MS", s.frames / seconds, 1000.0 * s.cost / s.frames);
        s.since = now;
        s.frames = 0;
        s.cost = 0.0;
    }
}

void draw(unsigned framebuffer, int width, int height) {
    if (s.text.empty() || width <= 0 || height <= 0) {
        return;
    }
    int w = s.width;
    int h = kRows + 2 * kMargin;
    if (s.texture == 0) {
        glGenTextures(1, &s.texture);
        glGenFramebuffers(1, &s.framebuffer);
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, s.framebuffer);
    // The texture is sent again only when the text changed.
    if (s.text != s.drawn) {
        const std::vector<std::uint8_t> rgba = picture(s.text, w, h);
        glBindTexture(GL_TEXTURE_2D, s.texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(GL_RGBA8), w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     rgba.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(GL_NEAREST));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(GL_NEAREST));
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s.texture, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        s.drawn = s.text;
        s.width = w;
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    // Whole pixels of the picture, two to a 448-line frame's line, so the glyphs stay sharp.
    const int scale = std::max(1, (height + 224) / 448 * 2);
    const int x0 = 4 * scale;
    const int y1 = height - 4 * scale;
    // The picture's first row is its top; GL's framebuffer counts rows from the bottom.
    glBlitFramebuffer(0, 0, w, h, x0, y1, x0 + w * scale, y1 - h * scale, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void release() {
    if (s.texture != 0) {
        glDeleteTextures(1, &s.texture);
        glDeleteFramebuffers(1, &s.framebuffer);
    }
    s = State{};
}

}  // namespace openrac::frontend::fps_counter
