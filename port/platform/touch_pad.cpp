// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "platform/touch_pad.h"

#include <algorithm>
#include <cmath>

namespace openrac::platform {
namespace {

// A button answers a little outside its circle: thumbs are wider than the drawing.
constexpr float kButtonReach = 1.3f;
// A stick is picked up a little outside its circle too, and its knob goes to its edge.
constexpr float kStickReach = 1.5f;

using Kind = TouchControl::Kind;

TouchControl stick(Kind kind, float x, float y, float radius) {
    return TouchControl{kind, Button::Cross, x, y, radius};
}

TouchControl button(Button b, float x, float y, float radius) {
    return TouchControl{Kind::Button, b, x, y, radius};
}

std::uint8_t axis_byte(float offset, float radius) {
    const float v = std::clamp(offset / radius, -1.0f, 1.0f);
    return static_cast<std::uint8_t>(std::lround(127.5f + v * 127.5f));
}

}  // namespace

TouchPad::TouchPad() {
    // The left thumb moves Ratchet (left stick, d-pad above it), the right one jumps and attacks
    // (the face buttons) and turns the camera (right stick, below them on the inside); the
    // shoulders along the top edge, where the index fingers are.
    m_controls = {
        stick(Kind::LeftStick, 0.15f, 0.70f, 0.16f),
        stick(Kind::RightStick, 0.68f, 0.80f, 0.11f),
        button(Button::Up, 0.15f, 0.21f, 0.05f),
        button(Button::Down, 0.15f, 0.39f, 0.05f),
        button(Button::Left, 0.10f, 0.30f, 0.05f),
        button(Button::Right, 0.20f, 0.30f, 0.05f),
        button(Button::Triangle, 0.88f, 0.48f, 0.065f),
        button(Button::Cross, 0.88f, 0.76f, 0.065f),
        button(Button::Square, 0.81f, 0.62f, 0.065f),
        button(Button::Circle, 0.95f, 0.62f, 0.065f),
        button(Button::L2, 0.06f, 0.07f, 0.05f),
        button(Button::L1, 0.16f, 0.07f, 0.05f),
        button(Button::R1, 0.84f, 0.07f, 0.05f),
        button(Button::R2, 0.94f, 0.07f, 0.05f),
        button(Button::Select, 0.43f, 0.07f, 0.04f),
        button(Button::Start, 0.57f, 0.07f, 0.04f),
    };
}

void TouchPad::set_aspect(float aspect) {
    if (aspect > 0.0f) {
        m_aspect = aspect;
    }
}

float TouchPad::distance(const TouchControl& c, float x, float y) const {
    // Across the window in window heights: x times the aspect.
    const float dx = (x - c.x) * m_aspect;
    const float dy = y - c.y;
    return std::sqrt(dx * dx + dy * dy);
}

TouchPad::Finger* TouchPad::find(std::uint64_t id) {
    const auto it = std::find_if(m_fingers.begin(), m_fingers.end(), [id](const Finger& f) { return f.id == id; });
    return it != m_fingers.end() ? &*it : nullptr;
}

void TouchPad::finger_down(std::uint64_t finger, float x, float y) {
    m_visible = true;
    m_touched = true;
    Finger f{finger, x, y, -1};
    for (std::size_t i = 0; i < m_controls.size(); ++i) {
        const TouchControl& c = m_controls[i];
        if (c.kind == Kind::Button) {
            continue;
        }
        const bool taken = std::any_of(m_fingers.begin(), m_fingers.end(), [i](const Finger& other) {
            return other.stick == static_cast<int>(i);
        });
        if (!taken && distance(c, x, y) <= c.radius * kStickReach) {
            f.stick = static_cast<int>(i);
            break;
        }
    }
    if (Finger* old = find(finger)) {
        *old = f;
    } else {
        m_fingers.push_back(f);
    }
}

void TouchPad::finger_move(std::uint64_t finger, float x, float y) {
    if (Finger* f = find(finger)) {
        f->x = x;
        f->y = y;
    }
}

void TouchPad::finger_up(std::uint64_t finger) {
    std::erase_if(m_fingers, [finger](const Finger& f) { return f.id == finger; });
}

void TouchPad::release_all() {
    m_fingers.clear();
}

void TouchPad::idle(std::uint64_t now_ms) {
    if (m_touched || !m_fingers.empty()) {
        m_touched = false;
        m_last_touch_ms = now_ms;
    } else if (m_visible && now_ms - m_last_touch_ms >= kIdleHideMs) {
        m_visible = false;
    }
}

bool TouchPad::held(std::size_t index) const {
    if (index >= m_controls.size()) {
        return false;
    }
    const TouchControl& c = m_controls[index];
    return std::any_of(m_fingers.begin(), m_fingers.end(), [&](const Finger& f) {
        if (c.kind != Kind::Button) {
            return f.stick == static_cast<int>(index);
        }
        return f.stick < 0 && distance(c, f.x, f.y) <= c.radius * kButtonReach;
    });
}

void TouchPad::knob(std::size_t index, float& dx, float& dy) const {
    dx = 0.0f;
    dy = 0.0f;
    if (index >= m_controls.size()) {
        return;
    }
    const TouchControl& c = m_controls[index];
    for (const Finger& f : m_fingers) {
        if (f.stick == static_cast<int>(index)) {
            dx = (f.x - c.x) * m_aspect;
            dy = f.y - c.y;
            const float length = std::sqrt(dx * dx + dy * dy);
            if (length > c.radius) {
                dx *= c.radius / length;
                dy *= c.radius / length;
            }
            return;
        }
    }
}

void TouchPad::apply(Pad& pad) const {
    for (std::size_t i = 0; i < m_controls.size(); ++i) {
        if (!held(i)) {
            continue;
        }
        const TouchControl& c = m_controls[i];
        if (c.kind == Kind::Button) {
            pad.set(c.button, true);
            continue;
        }
        float dx = 0.0f;
        float dy = 0.0f;
        knob(i, dx, dy);
        if (c.kind == Kind::LeftStick) {
            pad.left_x = axis_byte(dx, c.radius);
            pad.left_y = axis_byte(dy, c.radius);
        } else {
            pad.right_x = axis_byte(dx, c.radius);
            pad.right_y = axis_byte(dy, c.radius);
        }
    }
}

}  // namespace openrac::platform
