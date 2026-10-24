// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The on-screen pad drawn (renderer/ui/touch_overlay.h) on a real context, desktop GL or ES
// (touch_overlay_es): its shader builds, nothing is drawn before the screen is touched, and a held
// button is drawn brighter than one let go. Skipped (exit 77) where no context can be made.

#include <cstdio>
#include <string>
#include <vector>

#include "check.h"
#include "renderer/gl.h"
#include "renderer/ui/touch_overlay.h"
#include "renderer_gl_context.h"

namespace {

using namespace openrac;
using namespace openrac::gl;

constexpr int kWidth = 320;
constexpr int kHeight = 180;

// The window's pixel under window units (x, y), as a sum of its red, green and blue.
int brightness(float x, float y) {
    unsigned char rgba[4] = {};
    glReadPixels(static_cast<GLint>(x * kWidth), static_cast<GLint>((1.0f - y) * kHeight), 1, 1, GL_RGBA,
                 GL_UNSIGNED_BYTE, rgba);
    return rgba[0] + rgba[1] + rgba[2];
}

void clear() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

}  // namespace

int main() {
    test::GlContext context;
    std::string why;
    if (!context.open(why, kWidth, kHeight)) {
        std::printf("SKIP: no OpenGL context: %s\n", why.c_str());
        return test::kSkip;
    }
    platform::TouchPad touch;
    const platform::TouchControl* cross = nullptr;
    for (const platform::TouchControl& c : touch.controls()) {
        if (c.kind == platform::TouchControl::Kind::Button && c.button == platform::Button::Cross) {
            cross = &c;
        }
    }
    CHECK(cross != nullptr);
    if (cross == nullptr) {
        return test::result();
    }
    // The ring's edge, where every button draws whatever its symbol.
    const float edge_x = cross->x + cross->radius * 0.92f * kHeight / kWidth;

    clear();
    renderer::ui::touch_overlay::draw(touch, kWidth, kHeight);
    CHECK(brightness(edge_x, cross->y) == 0);  // not touched yet: nothing drawn

    touch.finger_down(1, 0.5f, 0.5f);  // touched away from every control: shown, nothing held
    clear();
    renderer::ui::touch_overlay::draw(touch, kWidth, kHeight);
    const int released = brightness(edge_x, cross->y);
    CHECK(released > 0);

    touch.finger_down(2, cross->x, cross->y);
    clear();
    renderer::ui::touch_overlay::draw(touch, kWidth, kHeight);
    const int held = brightness(edge_x, cross->y);
    CHECK(held > released);
    CHECK(glGetError() == GL_NO_ERROR);

    // Frame after frame, as the game draws it: the picture blitted into the window, the pad over
    // it, shown. The pad looks the same in each (on Vulkan it flickered on a phone).
    GLuint picture = 0;
    GLuint framebuffer = 0;
    glGenTextures(1, &picture);
    glBindTexture(GL_TEXTURE_2D, picture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, picture, 0);
    for (int frame = 0; frame < 4; ++frame) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        clear();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
        glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        renderer::ui::touch_overlay::draw(touch, kWidth, kHeight);
        CHECK(brightness(edge_x, cross->y) == held);
        context.window().swap();
    }
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &picture);
    CHECK(glGetError() == GL_NO_ERROR);

    renderer::ui::touch_overlay::release();
    return test::result();
}
