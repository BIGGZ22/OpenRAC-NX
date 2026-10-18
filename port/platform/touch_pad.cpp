// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "platform/touch_pad.h"

#include <algorithm>
#include <cmath>

namespace openrac::platform {
namespace {

// A button answers a little outside its outline: thumbs are wider than the drawing.
constexpr float kButtonReach = 0.025f;
// A stick is picked up a little outside its circle too, and its knob goes to its edge.
constexpr float kStickReach = 1.5f;
// Where a thumb brings the left stick to it: the lower left, below the d-pad, left of this.
constexpr float kStickZoneTop = 0.42f;
constexpr float kStickZoneRight = 0.42f;

using Kind = TouchControl::Kind;

std::uint8_t axis_byte(float offset, float radius) {
    const float v = std::clamp(offset / radius, -1.0f, 1.0f);
    return static_cast<std::uint8_t>(std::lround(127.5f + v * 127.5f));
}

}  // namespace

TouchPad::TouchPad() {
    // The left thumb moves Ratchet (the left stick, the d-pad above it); the right one jumps and
    // attacks (the face buttons, Cross the largest, where the thumb rests) and turns the camera
    // (the right stick, inside and below them); the shoulders along the top, under the index
    // fingers. Offsets and sizes in window heights, from the nearer edge.
    using From = Place::From;
    struct Item {
        Kind kind;
        Button button;
        From from;
        float offset, y, radius, stretch;
    };
    constexpr float kFace = 0.105f;  // the face buttons' distance from their diamond's centre
    constexpr float kArm = 0.078f;   // the d-pad's arms' from its centre
    const Item items[] = {
        {Kind::LeftStick, Button::Cross, From::Left, 0.27f, 0.69f, 0.15f, 1.0f},
        {Kind::RightStick, Button::Cross, From::Right, 0.56f, 0.80f, 0.105f, 1.0f},
        {Kind::Button, Button::Up, From::Left, 0.20f, 0.30f - kArm, 0.052f, 1.0f},
        {Kind::Button, Button::Down, From::Left, 0.20f, 0.30f + kArm, 0.052f, 1.0f},
        {Kind::Button, Button::Left, From::Left, 0.20f - kArm, 0.30f, 0.052f, 1.0f},
        {Kind::Button, Button::Right, From::Left, 0.20f + kArm, 0.30f, 0.052f, 1.0f},
        {Kind::Button, Button::Triangle, From::Right, 0.24f, 0.52f - kFace, 0.066f, 1.0f},
        {Kind::Button, Button::Cross, From::Right, 0.24f, 0.52f + kFace, 0.076f, 1.0f},
        {Kind::Button, Button::Square, From::Right, 0.24f + kFace, 0.52f, 0.066f, 1.0f},
        {Kind::Button, Button::Circle, From::Right, 0.24f - kFace, 0.52f, 0.066f, 1.0f},
        {Kind::Button, Button::L2, From::Left, 0.13f, 0.075f, 0.045f, 1.75f},
        {Kind::Button, Button::L1, From::Left, 0.32f, 0.075f, 0.045f, 1.75f},
        {Kind::Button, Button::R1, From::Right, 0.32f, 0.075f, 0.045f, 1.75f},
        {Kind::Button, Button::R2, From::Right, 0.13f, 0.075f, 0.045f, 1.75f},
        {Kind::Button, Button::Select, From::Middle, -0.14f, 0.075f, 0.032f, 1.6f},
        {Kind::Button, Button::Start, From::Middle, 0.14f, 0.075f, 0.032f, 1.6f},
        {Kind::Menu, Button::Cross, From::Middle, 0.0f, 0.075f, 0.036f, 1.0f},
    };
    for (const Item& item : items) {
        m_controls.push_back(TouchControl{item.kind, item.button, 0.0f, item.y, item.radius, item.stretch});
        m_places.push_back(Place{item.from, item.offset, item.y});
    }
    layout();
}

void TouchPad::set_aspect(float aspect) {
    if (aspect > 0.0f && aspect != m_aspect) {
        m_aspect = aspect;
        layout();
    }
}

void TouchPad::set_safe_area(float left, float right) {
    left = std::max(left, 0.0f);
    right = std::max(right, 0.0f);
    if (left != m_safe_left || right != m_safe_right) {
        m_safe_left = left;
        m_safe_right = right;
        layout();
    }
}

void TouchPad::place(std::size_t index) {
    const Place& p = m_places[index];
    TouchControl& c = m_controls[index];
    float across = 0.0f;  // in window heights from the left edge
    switch (p.from) {
        case Place::From::Left: across = m_safe_left + p.offset; break;
        case Place::From::Middle: across = m_aspect * 0.5f + p.offset; break;
        case Place::From::Right: across = m_aspect - m_safe_right - p.offset; break;
    }
    c.x = across / m_aspect;
    c.y = p.y;
}

void TouchPad::layout() {
    for (std::size_t i = 0; i < m_controls.size(); ++i) {
        // A stick a thumb holds stays under it.
        const bool held_stick = std::any_of(m_fingers.begin(), m_fingers.end(), [i](const Finger& f) {
            return f.stick == static_cast<int>(i);
        });
        if (!held_stick) {
            place(i);
        }
    }
}

void TouchPad::bring_stick(std::size_t index, float x, float y) {
    TouchControl& c = m_controls[index];
    const float r = c.radius;
    const float across = std::clamp(x * m_aspect, m_safe_left + r, m_aspect * kStickZoneRight + r);
    c.x = across / m_aspect;
    c.y = std::clamp(y, kStickZoneTop, 1.0f - r);
}

float TouchPad::distance(const TouchControl& c, float x, float y) const {
    // Across the window in window heights: x times the aspect. A pill's straight middle counts as
    // its centre.
    const float straight = c.radius * (c.stretch - 1.0f);
    const float dx = std::max(std::abs((x - c.x) * m_aspect) - straight, 0.0f);
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
    if (Finger* old = find(finger)) {
        finger_up(old->id);
    }
    Finger f{finger, x, y, -1};
    const auto taken = [this](std::size_t i) {
        return std::any_of(m_fingers.begin(), m_fingers.end(), [i](const Finger& other) {
            return other.stick == static_cast<int>(i);
        });
    };
    bool on_button = false;
    for (std::size_t i = 0; i < m_controls.size(); ++i) {
        const TouchControl& c = m_controls[i];
        if (c.kind == Kind::Menu) {
            if (m_menu_button && distance(c, x, y) <= c.radius + kButtonReach) {
                m_menu_pressed = true;
                on_button = true;
            }
            continue;
        }
        if (c.kind == Kind::Button) {
            on_button = on_button || distance(c, x, y) <= c.radius + kButtonReach;
            continue;
        }
        if (!taken(i) && distance(c, x, y) <= c.radius * kStickReach) {
            f.stick = static_cast<int>(i);
            break;
        }
    }
    if (f.stick < 0 && !on_button && y >= kStickZoneTop && x * m_aspect <= m_aspect * kStickZoneRight) {
        for (std::size_t i = 0; i < m_controls.size(); ++i) {
            if (m_controls[i].kind == Kind::LeftStick && !taken(i)) {
                bring_stick(i, x, y);
                f.stick = static_cast<int>(i);
                break;
            }
        }
    }
    m_fingers.push_back(f);
}

void TouchPad::finger_move(std::uint64_t finger, float x, float y) {
    if (Finger* f = find(finger)) {
        f->x = x;
        f->y = y;
    }
}

void TouchPad::finger_up(std::uint64_t finger) {
    std::erase_if(m_fingers, [finger](const Finger& f) { return f.id == finger; });
    layout();  // a stick let go goes home
}

void TouchPad::release_all() {
    m_fingers.clear();
    layout();
}

void TouchPad::set_menu_button(bool on) {
    m_menu_button = on;
    if (!on) {
        m_menu_pressed = false;
    }
}

bool TouchPad::take_menu_press() {
    const bool pressed = m_menu_pressed;
    m_menu_pressed = false;
    return pressed;
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
    if (c.kind == Kind::Menu && !m_menu_button) {
        return false;
    }
    return std::any_of(m_fingers.begin(), m_fingers.end(), [&](const Finger& f) {
        if (c.kind == Kind::Menu) {
            return f.stick < 0 && distance(c, f.x, f.y) <= c.radius + kButtonReach;
        }
        if (c.kind != Kind::Button) {
            return f.stick == static_cast<int>(index);
        }
        return f.stick < 0 && distance(c, f.x, f.y) <= c.radius + kButtonReach;
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
        if (c.kind == Kind::Menu) {
            continue;  // the port's, not the game's
        }
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
