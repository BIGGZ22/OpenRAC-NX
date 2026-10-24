// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Tests the Vulkan backend's one copy to the screen (present-single-copy, docs/port/RENDER_PERF.md
// "R-07"): the frontend's present() clears the window black and blits the frame's picture into a
// box of it, and the backend then copies the window into the screen. With the entry on, that blit
// waits and the present makes one blit of the picture straight into the screen.
//
// A swapchain cannot be read back, so vkgl::test_present_paths draws the copy that waits into two
// stand-ins of the screen, once the single way and once the two-step way, and the test compares
// them byte for byte: the composed blit is the only new arithmetic. Then a draw into the window
// after the blit makes the copy at once, and the window reads back as it does with the entry off.
// The picture is four flat colours, so a blit's filtering shows wherever the paths differ.
//
// Skipped (77) where Vulkan cannot open.

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "renderer/gl.h"
#include "renderer/render_opt.h"
#include "renderer/vulkan/vkgl.h"
#include "tests/check.h"
#include "tests/renderer_gl_context.h"

namespace {

using namespace openrac::gl;
namespace render_opt = openrac::renderer::render_opt;

// The window (the default framebuffer) and the picture, of other sizes so the blit scales.
constexpr int kWindowWidth = 64;
constexpr int kWindowHeight = 48;
constexpr int kPictureWidth = 40;
constexpr int kPictureHeight = 24;

// Where the frontend's box goes: inset from the window, as a letterboxed picture is.
constexpr int kBoxLeft = 6;
constexpr int kBoxBottom = 9;
constexpr int kBoxRight = 58;
constexpr int kBoxTop = 39;

// A picture: a framebuffer object with an RGBA8 texture.
struct Picture {
    GLuint texture = 0;
    GLuint framebuffer = 0;
};

// Makes the picture and paints it four flat colours, a quarter each, red at the bottom left.
Picture make_picture() {
    Picture p;
    glGenTextures(1, &p.texture);
    glBindTexture(GL_TEXTURE_2D, p.texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        kPictureWidth,
        kPictureHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        nullptr
    );
    glGenFramebuffers(1, &p.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, p.framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, p.texture, 0);
    glViewport(0, 0, kPictureWidth, kPictureHeight);
    // One colour per quarter: x then y, in the order of a reading from the bottom left.
    constexpr std::array<std::array<float, 4>, 4> kQuarters = {{
        {1.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
    }};
    glEnable(GL_SCISSOR_TEST);

    // Each quarter cleared to its colour inside its scissor box.
    for (std::size_t n = 0; n < kQuarters.size(); ++n) {
        const int x = static_cast<int>(n % 2) * kPictureWidth / 2;
        const int y = static_cast<int>(n / 2) * kPictureHeight / 2;
        glScissor(x, y, kPictureWidth / 2, kPictureHeight / 2);
        glClearColor(kQuarters[n][0], kQuarters[n][1], kQuarters[n][2], kQuarters[n][3]);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    glDisable(GL_SCISSOR_TEST);
    return p;
}

// Does what the frontend's present() does: the window cleared black, the picture blitted into
// the box, its first row at the top when `flip`, with `filter`.
void blit_to_window(const Picture& p, bool flip, GLenum filter) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, p.framebuffer);
    const int y0 = flip ? kBoxTop : kBoxBottom;
    const int y1 = flip ? kBoxBottom : kBoxTop;
    glBlitFramebuffer(
        0,
        0,
        kPictureWidth,
        kPictureHeight,
        kBoxLeft,
        y0,
        kBoxRight,
        y1,
        GL_COLOR_BUFFER_BIT,
        filter
    );
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

// Returns whether `pixels` holds a texel that is not black: the picture arrived.
bool shows_picture(const std::vector<std::uint8_t>& pixels) {
    // Any colour channel of any texel (four bytes a texel; the fourth is alpha).
    for (std::size_t n = 0; n < pixels.size(); ++n) {
        // Alpha is 255 everywhere, black included.
        if (n % 4 != 3 && pixels[n] != 0) {
            return true;
        }
    }

    return false;
}

// Tests that the one blit into the screen gives the bytes the two copies give, for a blit
// `flip`ped or not, with `filter`.
void test_one_copy_is_the_two_copies(const Picture& p, bool flip, GLenum filter) {
    blit_to_window(p, flip, filter);
    std::vector<std::uint8_t> single;
    std::vector<std::uint8_t> two_step;
    CHECK(openrac::vkgl::test_present_paths(single, two_step));
    CHECK(!single.empty() && single.size() == two_step.size());
    CHECK(single == two_step);
    CHECK(shows_picture(single));
}

// Draws the picture into the window as the frontend does, then a square over it (a stand-in for
// an overlay drawn after the present's blit), and returns the window read back.
std::vector<std::uint8_t> window_with_overlay(const Picture& p) {
    blit_to_window(p, true, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_SCISSOR_TEST);
    // The overlay: a square across the box's edge, so it covers both picture and border.
    glScissor(2, 2, 16, 16);
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
    std::vector<std::uint8_t> pixels(static_cast<std::size_t>(kWindowWidth * kWindowHeight * 4));
    glReadPixels(0, 0, kWindowWidth, kWindowHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    return pixels;
}

// Tests that a draw into the window after the blit makes the copy first: the window reads back
// as it does with present-single-copy off, and no copy waits any more.
void test_a_draw_after_the_blit_makes_the_copy(const Picture& p) {
    const std::vector<std::uint8_t> on = window_with_overlay(p);
    std::vector<std::uint8_t> unused;
    CHECK(!openrac::vkgl::test_present_paths(unused, unused));
    render_opt::set(render_opt::Opt::PresentSingleCopy, false);
    const std::vector<std::uint8_t> off = window_with_overlay(p);
    render_opt::set(render_opt::Opt::PresentSingleCopy, true);
    CHECK(on == off);
    CHECK(shows_picture(on));
}

// Returns the full-screen copies one frame of the frontend's present makes: the blit into the
// window, then the window shown (the copy to the screen where the window has a swapchain).
std::uint64_t copies_of_a_frame(openrac::test::GlContext& context, const Picture& p) {
    const std::uint64_t before = openrac::vkgl::submission_counters().fullscreen_copies;
    blit_to_window(p, true, GL_LINEAR);
    context.window().swap();
    return openrac::vkgl::submission_counters().fullscreen_copies - before;
}

// Tests that a frame shown with present-single-copy makes no more full-screen copies than without,
// and prints both counts for the report (one fewer where the window has a swapchain).
void test_a_frame_copies_less(openrac::test::GlContext& context, const Picture& p) {
    const std::uint64_t on = copies_of_a_frame(context, p);
    render_opt::set(render_opt::Opt::PresentSingleCopy, false);
    const std::uint64_t off = copies_of_a_frame(context, p);
    render_opt::set(render_opt::Opt::PresentSingleCopy, true);
    std::printf(
        "vk_fullscreen_copies a frame: %llu on, %llu off\n",
        static_cast<unsigned long long>(on),
        static_cast<unsigned long long>(off)
    );
    CHECK(on <= off);
}

}  // namespace

int main() {
    openrac::test::GlContext context;
    std::string error;

    // No Vulkan here (or it fell back to OpenGL): nothing of the backend to test.
    if (!context.open(error, kWindowWidth, kWindowHeight) || !openrac::vkgl::active()) {
        std::printf("SKIP: Vulkan unavailable: %s\n", error.c_str());
        return openrac::test::kSkip;
    }

    // The paths under test, whatever OPENRAC_RENDER_OPT_OFF says: the copy waits only when both
    // are on (it needs attachment-discard's deferred passes).
    render_opt::set(render_opt::Opt::AttachmentDiscard, true);
    render_opt::set(render_opt::Opt::PresentSingleCopy, true);
    const Picture picture = make_picture();
    test_one_copy_is_the_two_copies(picture, true, GL_LINEAR);
    test_one_copy_is_the_two_copies(picture, false, GL_NEAREST);
    test_a_draw_after_the_blit_makes_the_copy(picture);
    test_a_frame_copies_less(context, picture);
    CHECK(openrac::vkgl::submission_counters().validation_errors == 0);
    return openrac::test::result();
}
