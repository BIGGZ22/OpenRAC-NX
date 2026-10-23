// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "platform/touch_pad.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <format>

namespace openrac::platform {
namespace {

// A button answers a little outside its outline: thumbs are wider than the drawing.
constexpr float kButtonReach = 0.025f;
// A stick is picked up a little outside its circle too, and its knob goes to its edge.
constexpr float kStickReach = 1.5f;
// Where a thumb brings the left stick to it: the lower left, below the d-pad, left of this.
constexpr float kStickZoneTop = 0.42f;
constexpr float kStickZoneRight = 0.42f;
// Where a finger turns the camera: the right of the screen, from this across, off the buttons.
constexpr float kLookZoneLeft = 0.5f;
// The camera: an invisible stick under the finger that lands there. Pushed this many window
// heights from where the finger landed, it is pushed all the way (at the pad's own speed; the
// player's speed shortens or lengthens the push), and it keeps turning the camera as long as the
// finger stays out there, as a stick held over does. Pushed further, its centre follows the
// finger, so that a finger coming back turns the camera the other way at once. Within kLookDead
// of the centre it is still; past it the push starts at kLookFloor, past the game's dead zone, so
// that a small push turns the camera slowly rather than not at all.
constexpr float kLookReach = 0.1f;
constexpr float kLookDead = 0.12f;
constexpr float kLookFloor = 0.22f;
// How far the player may move a group from its place, in window heights.
constexpr float kMaxOffset = 2.0f;
// A group's centre keeps this far inside the window.
constexpr float kInside = 0.04f;

using Kind = TouchControl::Kind;

std::uint8_t axis_byte(float v) {
    v = std::clamp(v, -1.0f, 1.0f);
    return static_cast<std::uint8_t>(std::lround(127.5f + v * 127.5f));
}

constexpr const char* kGroupNames[] =
    {"stick", "dpad", "face", "l1", "l2", "r1", "r2", "select", "start", "menu"};
static_assert(std::size(kGroupNames) == kTouchGroups, "a name for each TouchGroup");

}  // namespace

const char* TouchPad::group_name(TouchGroup group) {
    const auto i = static_cast<std::size_t>(group);
    return i < kTouchGroups ? kGroupNames[i] : "";
}

TouchPad::TouchPad() {
    // The left thumb moves Ratchet (the left stick, the d-pad above it); the right one jumps and
    // attacks (the face buttons, all of one size as on the pad) and turns the camera on the
    // screen around them; the shoulders along the top, under the index fingers. Each group's
    // centre from the nearer edge, its controls around it; in window heights at scale 1.
    using From = Place::From;
    using G = TouchGroup;
    const auto group = [this](G g, From from, float offset, float y) {
        m_places[static_cast<std::size_t>(g)] = Place{from, offset, y};
    };
    group(G::Stick, From::Left, 0.27f, 0.69f);
    group(G::Dpad, From::Left, 0.20f, 0.36f);
    group(G::Face, From::Right, 0.26f, 0.64f);
    // The shoulders a little below the top, where the game draws its health and bolts.
    group(G::L2, From::Left, 0.13f, 0.15f);
    group(G::L1, From::Left, 0.36f, 0.15f);
    group(G::R1, From::Right, 0.36f, 0.15f);
    group(G::R2, From::Right, 0.13f, 0.15f);
    group(G::Select, From::Middle, -0.14f, 0.075f);
    group(G::Start, From::Middle, 0.14f, 0.075f);
    group(G::Menu, From::Middle, 0.0f, 0.075f);

    struct Item {
        Kind kind;
        Button button;
        G group;
        float dx, dy, radius, stretch;
    };
    constexpr float kFace = 0.105f;  // the face buttons' distance from their diamond's centre
    constexpr float kArm = 0.078f;   // the d-pad's arms' from its centre
    const Item items[] = {
        {Kind::LeftStick, Button::Cross, G::Stick, 0.0f, 0.0f, 0.15f, 1.0f},
        {Kind::Button, Button::Up, G::Dpad, 0.0f, -kArm, 0.052f, 1.0f},
        {Kind::Button, Button::Down, G::Dpad, 0.0f, kArm, 0.052f, 1.0f},
        {Kind::Button, Button::Left, G::Dpad, -kArm, 0.0f, 0.052f, 1.0f},
        {Kind::Button, Button::Right, G::Dpad, kArm, 0.0f, 0.052f, 1.0f},
        {Kind::Button, Button::Triangle, G::Face, 0.0f, -kFace, 0.07f, 1.0f},
        {Kind::Button, Button::Cross, G::Face, 0.0f, kFace, 0.07f, 1.0f},
        {Kind::Button, Button::Square, G::Face, -kFace, 0.0f, 0.07f, 1.0f},
        {Kind::Button, Button::Circle, G::Face, kFace, 0.0f, 0.07f, 1.0f},
        {Kind::Button, Button::L2, G::L2, 0.0f, 0.0f, 0.045f, 1.75f},
        {Kind::Button, Button::L1, G::L1, 0.0f, 0.0f, 0.045f, 1.75f},
        {Kind::Button, Button::R1, G::R1, 0.0f, 0.0f, 0.045f, 1.75f},
        {Kind::Button, Button::R2, G::R2, 0.0f, 0.0f, 0.045f, 1.75f},
        {Kind::Button, Button::Select, G::Select, 0.0f, 0.0f, 0.032f, 1.6f},
        {Kind::Button, Button::Start, G::Start, 0.0f, 0.0f, 0.032f, 1.6f},
        {Kind::Menu, Button::Cross, G::Menu, 0.0f, 0.0f, 0.036f, 1.0f},
    };
    for (const Item& item : items) {
        m_controls.push_back(
            TouchControl{item.kind, item.button, item.group, 0.0f, 0.0f, item.radius, item.stretch}
        );
        m_members.push_back(Member{item.dx, item.dy, item.radius});
    }
    layout_controls();
}

void TouchPad::set_aspect(float aspect) {
    if (aspect > 0.0f && aspect != m_aspect) {
        m_aspect = aspect;
        layout_controls();
    }
}

void TouchPad::set_safe_area(float left, float right) {
    left = std::max(left, 0.0f);
    right = std::max(right, 0.0f);
    if (left != m_safe_left || right != m_safe_right) {
        m_safe_left = left;
        m_safe_right = right;
        layout_controls();
    }
}

void TouchPad::set_scale(float scale) {
    scale = std::isfinite(scale) ? std::clamp(scale, kMinScale, kMaxScale) : kDefaultScale;
    if (scale != m_scale) {
        m_scale = scale;
        layout_controls();
    }
}

void TouchPad::set_look_speed(float speed) {
    m_look_speed = std::isfinite(speed) ? std::clamp(speed, 0.25f, 3.0f) : 1.0f;
}

void TouchPad::set_layout(std::string_view text) {
    m_offsets = {};
    std::size_t at = 0;
    while (at <= text.size()) {
        const std::size_t end = std::min(text.find(',', at), text.size());
        const std::string item(text.substr(at, end - at));
        char name[16] = {};
        float dx = 0.0f;
        float dy = 0.0f;
        char after = 0;
        if (std::sscanf(item.c_str(), " %15s %f %f %c", name, &dx, &dy, &after) == 3
            && std::isfinite(dx) && std::isfinite(dy)) {
            for (std::size_t g = 0; g < kTouchGroups; ++g) {
                if (std::string_view(name) == kGroupNames[g]) {
                    m_offsets[g] =
                        {std::clamp(dx, -kMaxOffset, kMaxOffset),
                         std::clamp(dy, -kMaxOffset, kMaxOffset)};
                }
            }
        }
        at = end + 1;
    }
    layout_controls();
}

std::string TouchPad::layout() const {
    std::string text;
    for (std::size_t g = 0; g < kTouchGroups; ++g) {
        const auto [dx, dy] = m_offsets[g];
        if (dx != 0.0f || dy != 0.0f) {
            text +=
                std::format("{}{} {:.4f} {:.4f}", text.empty() ? "" : ", ", kGroupNames[g], dx, dy);
        }
    }
    return text;
}

void TouchPad::reset_layout() {
    m_offsets = {};
    layout_controls();
}

void TouchPad::set_editing(bool on) {
    if (on == m_editing) {
        return;
    }
    m_editing = on;
    m_fingers.clear();  // a finger that came down as one thing is not the other's
    m_look_x = m_look_y = 0.0f;
    m_visible = m_visible || on;
    layout_controls();
}

void TouchPad::anchor(TouchGroup group, float& across, float& down) const {
    const auto g = static_cast<std::size_t>(group);
    const Place& p = m_places[g];
    switch (p.from) {
        case Place::From::Left:
            across = m_safe_left + p.offset * m_scale;
            break;
        case Place::From::Middle:
            across = m_aspect * 0.5f + p.offset * m_scale;
            break;
        case Place::From::Right:
            across = m_aspect - m_safe_right - p.offset * m_scale;
            break;
    }
    // Toward the nearer of the top and the bottom as the controls get smaller.
    down = p.y < 0.5f ? p.y * m_scale : 1.0f - (1.0f - p.y) * m_scale;
    across = std::clamp(across + m_offsets[g][0], kInside, std::max(kInside, m_aspect - kInside));
    down = std::clamp(down + m_offsets[g][1], kInside, 1.0f - kInside);
}

void TouchPad::place(std::size_t index) {
    TouchControl& c = m_controls[index];
    const Member& m = m_members[index];
    float across = 0.0f;
    float down = 0.0f;
    anchor(c.group, across, down);
    c.x = (across + m.dx * m_scale) / m_aspect;
    c.y = down + m.dy * m_scale;
    c.radius = m.radius * m_scale;
}

bool TouchPad::stick_taken(std::size_t index) const {
    return std::any_of(m_fingers.begin(), m_fingers.end(), [index](const Finger& f) {
        return f.role == Role::Stick && f.stick == static_cast<int>(index);
    });
}

void TouchPad::layout_controls() {
    for (std::size_t i = 0; i < m_controls.size(); ++i) {
        // A stick a thumb holds stays under it.
        if (!stick_taken(i)) {
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
    Finger f{finger, x, y};
    if (m_editing) {
        // The group under the finger, the nearest if two are close; nothing beside them.
        float nearest = 0.0f;
        for (const TouchControl& c : m_controls) {
            if (c.kind == Kind::Menu && !m_menu_button) {
                continue;
            }
            const float d = distance(c, x, y) - c.radius - kButtonReach;
            if (d <= 0.0f && (f.role != Role::Drag || d < nearest)) {
                f.role = Role::Drag;
                f.group = c.group;
                nearest = d;
            }
        }
        if (f.role == Role::Drag) {
            f.from_x = x;
            f.from_y = y;
            f.offset = m_offsets[static_cast<std::size_t>(f.group)];
        }
        m_fingers.push_back(f);
        return;
    }
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
        if (!stick_taken(i) && distance(c, x, y) <= c.radius * kStickReach) {
            f.role = Role::Stick;
            f.stick = static_cast<int>(i);
            break;
        }
    }
    if (f.role == Role::None && !on_button) {
        if (y >= kStickZoneTop && x <= kStickZoneRight) {
            for (std::size_t i = 0; i < m_controls.size(); ++i) {
                if (m_controls[i].kind == Kind::LeftStick && !stick_taken(i)) {
                    bring_stick(i, x, y);
                    f.role = Role::Stick;
                    f.stick = static_cast<int>(i);
                    break;
                }
            }
        } else if (x >= kLookZoneLeft) {
            f.role = Role::Look;
            f.from_x = x;
            f.from_y = y;
        }
    }
    m_fingers.push_back(f);
}

void TouchPad::finger_move(std::uint64_t finger, float x, float y) {
    Finger* f = find(finger);
    if (f == nullptr) {
        return;
    }
    f->x = x;
    f->y = y;
    if (f->role == Role::Look) {
        push_look(*f);
    } else if (f->role == Role::Drag) {
        const auto g = static_cast<std::size_t>(f->group);
        m_offsets[g] =
            {std::clamp(f->offset[0] + (x - f->from_x) * m_aspect, -kMaxOffset, kMaxOffset),
             std::clamp(f->offset[1] + (y - f->from_y), -kMaxOffset, kMaxOffset)};
        layout_controls();
    }
}

void TouchPad::finger_up(std::uint64_t finger) {
    std::erase_if(m_fingers, [finger](const Finger& f) { return f.id == finger; });
    // The camera stops with the finger lifted, or follows another still on it.
    m_look_x = m_look_y = 0.0f;
    for (Finger& f : m_fingers) {
        if (f.role == Role::Look) {
            push_look(f);
        }
    }
    layout_controls();  // a stick let go goes home
}

void TouchPad::release_all() {
    m_fingers.clear();
    m_look_x = m_look_y = 0.0f;
    layout_controls();
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
    if (m_touched || !m_fingers.empty() || m_editing) {
        m_touched = false;
        m_last_touch_ms = now_ms;
        m_visible = m_visible || m_editing;
    } else if (m_visible && now_ms - m_last_touch_ms >= kIdleHideMs) {
        m_visible = false;
    }
}

void TouchPad::push_look(Finger& f) {
    // In window heights across and down from the stick's centre, where the finger landed.
    const float reach = kLookReach / m_look_speed;
    float dx = (f.x - f.from_x) * m_aspect;
    float dy = f.y - f.from_y;
    const float length = std::sqrt(dx * dx + dy * dy);
    if (length > reach) {
        // Out past the edge: the centre is dragged after the finger.
        const float excess = (length - reach) / length;
        f.from_x += dx * excess / m_aspect;
        f.from_y += dy * excess;
        dx -= dx * excess;
        dy -= dy * excess;
    }
    m_look_x = dx / reach;
    m_look_y = dy / reach;
}

void TouchPad::look(float& x, float& y) const {
    x = 0.0f;
    y = 0.0f;
    const float length = std::sqrt(m_look_x * m_look_x + m_look_y * m_look_y);
    if (length <= kLookDead) {
        return;
    }
    const float out =
        kLookFloor
        + (1.0f - kLookFloor) * (std::min(length, 1.0f) - kLookDead) / (1.0f - kLookDead);
    x = m_look_x / length * out;
    y = m_look_y / length * out;
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
        switch (f.role) {
            case Role::Drag:
                return f.group == c.group;  // lit while it is being moved
            case Role::Stick:
                return f.stick == static_cast<int>(index);
            case Role::Look:
                return false;
            case Role::None:
                break;
        }
        return c.kind != Kind::LeftStick && !m_editing
               && distance(c, f.x, f.y) <= c.radius + kButtonReach;
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
        if (f.role == Role::Stick && f.stick == static_cast<int>(index)) {
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
    if (m_editing) {
        return;  // the fingers move the controls, not Ratchet
    }
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
        // The knob stays in its circle: squared as the DualShock 2's (pad.h), so a thumb pushed to
        // a diagonal runs.
        dx /= c.radius;
        dy /= c.radius;
        square_stick(dx, dy);
        pad.left_x = axis_byte(dx);
        pad.left_y = axis_byte(dy);
    }
    float lx = 0.0f;
    float ly = 0.0f;
    look(lx, ly);
    if (lx != 0.0f || ly != 0.0f) {
        square_stick(lx, ly);
        pad.right_x = axis_byte(lx);
        pad.right_y = axis_byte(ly);
    }
}

}  // namespace openrac::platform
