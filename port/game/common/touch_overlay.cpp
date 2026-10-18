// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "touch_overlay.h"

#include <algorithm>
#include <string>

#include "common/log.h"
#include "renderer/gl.h"
#include "renderer/shader.h"

namespace openrac::frontend::touch_overlay {
namespace {

using namespace openrac::gl;
using platform::Button;
using platform::TouchControl;

// One quad per control, its circle and symbol drawn by the fragment shader from the quad's own
// coordinates, so there is no texture to make.
constexpr const char* kVertex = R"glsl(#version 410 core
uniform vec4 box;  // the control's square in device coordinates: centre x, y, half width, half height
out vec2 v_at;     // -1..1 across the square
void main() {
    vec2 corner = vec2(float(gl_VertexID & 1), float((gl_VertexID >> 1) & 1)) * 2.0 - 1.0;
    v_at = corner;
    gl_Position = vec4(box.xy + corner * box.zw, 0.0, 1.0);
}
)glsl";

constexpr const char* kFragment = R"glsl(#version 410 core
in vec2 v_at;
uniform vec4 tint;    // the symbol's colour
uniform float held;   // 1 while a finger holds it
uniform int symbol;   // 0 none, 1 triangle, 2 circle, 3 cross, 4 square, 5 knob, 6 arrow (up)
uniform vec2 knob;    // a stick's knob, -1..1 of the circle
uniform float angle;  // the arrow's direction, in radians from up
out vec4 colour;

float ring(float d, float r, float w) { return 1.0 - smoothstep(w * 0.5, w * 0.5 + 0.04, abs(d - r)); }

void main() {
    float d = length(v_at);
    if (d > 1.0) { discard; }
    float base = mix(0.28, 0.5, held);
    vec4 c = vec4(0.08, 0.08, 0.1, base * 0.6);
    c = mix(c, vec4(1.0, 1.0, 1.0, base + 0.15), ring(d, 0.92, 0.08));
    float s = 0.0;
    if (symbol == 1) {
        vec2 p = v_at * vec2(1.0, -1.0) + vec2(0.0, 0.12);
        float e = max(abs(p.x) * 0.866 + p.y * 0.5, -p.y) - 0.25;
        s = 1.0 - smoothstep(0.0, 0.05, abs(e));
    } else if (symbol == 2) {
        s = ring(d, 0.38, 0.1);
    } else if (symbol == 3) {
        vec2 p = abs(mat2(0.7071, -0.7071, 0.7071, 0.7071) * v_at);
        s = (p.x < 0.07 || p.y < 0.07) && max(p.x, p.y) < 0.48 ? 1.0 : 0.0;
    } else if (symbol == 4) {
        vec2 p = abs(v_at);
        s = 1.0 - smoothstep(0.0, 0.05, abs(max(p.x, p.y) - 0.32));
    } else if (symbol == 5) {
        float k = length(v_at - knob * 0.55);
        s = 1.0 - smoothstep(0.38, 0.42, k);
    } else if (symbol == 6) {
        float ca = cos(angle), sa = sin(angle);
        vec2 p = mat2(ca, -sa, sa, ca) * v_at;
        p.y = -p.y;
        s = (p.y < 0.3 && p.y > -0.3 && abs(p.x) < (0.3 + p.y) * 0.8) ? 1.0 : 0.0;
    }
    c = mix(c, vec4(tint.rgb, max(tint.a * (0.55 + 0.45 * held), c.a)), s);
    colour = c;
}
)glsl";

struct State {
    renderer::Shader shader;
    GLuint vao = 0;
    bool failed = false;
};

State s;

struct Look {
    int symbol = 0;
    float r = 1.0f, g = 1.0f, b = 1.0f;
    float angle = 0.0f;
};

Look look(const TouchControl& c) {
    if (c.kind != TouchControl::Kind::Button) {
        return {5, 0.85f, 0.85f, 0.9f};
    }
    switch (c.button) {
        case Button::Triangle: return {1, 0.25f, 0.85f, 0.65f};
        case Button::Circle: return {2, 0.95f, 0.35f, 0.4f};
        case Button::Cross: return {3, 0.45f, 0.65f, 1.0f};
        case Button::Square: return {4, 0.95f, 0.5f, 0.85f};
        case Button::Up: return {6, 0.9f, 0.9f, 0.9f, 0.0f};
        case Button::Right: return {6, 0.9f, 0.9f, 0.9f, 1.5708f};
        case Button::Down: return {6, 0.9f, 0.9f, 0.9f, 3.1416f};
        case Button::Left: return {6, 0.9f, 0.9f, 0.9f, -1.5708f};
        default: return {0, 0.9f, 0.9f, 0.9f};
    }
}

bool ready() {
    if (s.failed) {
        return false;
    }
    if (s.vao == 0) {
        std::string error;
        if (!s.shader.build("touch overlay", kVertex, kFragment, error)) {
            log::warn("the on-screen pad cannot be drawn: {}", error);
            s.failed = true;
            return false;
        }
        glGenVertexArrays(1, &s.vao);
    }
    return true;
}

}  // namespace

void draw(platform::TouchPad& touch, int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }
    touch.set_aspect(static_cast<float>(width) / static_cast<float>(height));
    if (!touch.visible() || !ready()) {
        return;
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
    s.shader.use();
    glBindVertexArray(s.vao);
    const float aspect = touch.aspect();
    const auto controls = touch.controls();
    for (std::size_t i = 0; i < controls.size(); ++i) {
        const TouchControl& c = controls[i];
        const Look l = look(c);
        // Window units (0..1 down) to device coordinates (-1..1 up); the radius is in heights.
        glUniform4f(s.shader.uniform("box"), c.x * 2.0f - 1.0f, 1.0f - c.y * 2.0f, c.radius * 2.0f / aspect,
                    c.radius * 2.0f);
        glUniform4f(s.shader.uniform("tint"), l.r, l.g, l.b, 0.9f);
        glUniform1f(s.shader.uniform("held"), touch.held(i) ? 1.0f : 0.0f);
        glUniform1i(s.shader.uniform("symbol"), l.symbol);
        glUniform1f(s.shader.uniform("angle"), l.angle);
        float dx = 0.0f;
        float dy = 0.0f;
        touch.knob(i, dx, dy);
        glUniform2f(s.shader.uniform("knob"), dx / c.radius, -dy / c.radius);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

void release() {
    s.shader.release();
    if (s.vao != 0) {
        glDeleteVertexArrays(1, &s.vao);
    }
    s = State{};
}

}  // namespace openrac::frontend::touch_overlay
