// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The on-screen pad drawn (game/common/touch_overlay.h) on a real context, desktop GL or ES
// (touch_overlay_es): its shader builds, nothing is drawn before the screen is touched, and a held
// button is drawn brighter than one let go. Skipped (exit 77) where no context can be made.

#include <cstdio>
#include <string>
#include <vector>

#include "check.h"
#include "renderer/gl.h"
#include "renderer_gl_context.h"
#include "touch_overlay.h"

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
    frontend::touch_overlay::draw(touch, kWidth, kHeight);
    CHECK(brightness(edge_x, cross->y) == 0);  // not touched yet: nothing drawn

    touch.finger_down(1, 0.5f, 0.5f);  // touched away from every control: shown, nothing held
    clear();
    frontend::touch_overlay::draw(touch, kWidth, kHeight);
    const int released = brightness(edge_x, cross->y);
    CHECK(released > 0);

    touch.finger_down(2, cross->x, cross->y);
    clear();
    frontend::touch_overlay::draw(touch, kWidth, kHeight);
    const int held = brightness(edge_x, cross->y);
    CHECK(held > released);
    CHECK(glGetError() == GL_NO_ERROR);

    frontend::touch_overlay::release();
    return test::result();
}
