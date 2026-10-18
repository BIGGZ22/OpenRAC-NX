// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A DualShock 2 on a touch screen (Android phones and tablets; any touch screen SDL reports): two
// sticks, the d-pad, the four face buttons, the shoulders, Select and Start, laid out for a picture
// held sideways. Fingers come in as SDL gives them (0..1 across and down the window) and the pad's
// state goes out as a Pad's (pad.h), added to what the keyboard and a gamepad give port 0.
//
// A stick is held by the finger that touched inside it, and follows it until it lifts, even
// outside the circle; a button is held while any finger is on it. Nothing here touches SDL or GL:
// input.cpp feeds it events and the frontend draws controls() (game/common/touch_overlay.h).

#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "platform/pad.h"

namespace openrac::platform {

struct TouchControl {
    enum class Kind { LeftStick, RightStick, Button };
    Kind kind = Kind::Button;
    Button button = Button::Cross;  // for Kind::Button
    // The centre: x across the window (0..1), y down it (0..1); the radius in window heights.
    float x = 0.0f;
    float y = 0.0f;
    float radius = 0.0f;
};

class TouchPad {
public:
    TouchPad();

    // The window's width over its height: the controls keep their shape at any size.
    void set_aspect(float aspect);

    float aspect() const { return m_aspect; }

    void finger_down(std::uint64_t finger, float x, float y);
    void finger_move(std::uint64_t finger, float x, float y);
    void finger_up(std::uint64_t finger);

    // Every finger lifted (the window lost focus, the app went to the background).
    void release_all();

    // Adds what the fingers hold to `pad`: buttons pressed, a held stick's axes replacing the pad's.
    void apply(Pad& pad) const;

    // Hidden at first; shown once the screen has been touched, until another device is used
    // (hide()) or no finger has touched it for kIdleHideMs (idle()).
    bool visible() const { return m_visible; }

    void hide() { m_visible = false; }

    // Once a frame, with SDL's clock: puts the pad away when the screen has been left alone.
    static constexpr std::uint64_t kIdleHideMs = 10000;
    void idle(std::uint64_t now_ms);

    std::span<const TouchControl> controls() const { return m_controls; }

    // Whether control `index` is held now, and where a stick's knob is (offset from its centre in
    // window heights).
    bool held(std::size_t index) const;
    void knob(std::size_t index, float& dx, float& dy) const;

private:
    struct Finger {
        std::uint64_t id = 0;
        float x = 0.0f;
        float y = 0.0f;
        int stick = -1;  // the index of the stick control it holds, or -1
    };

    // Distance from a point to control `c`'s centre, in window heights.
    float distance(const TouchControl& c, float x, float y) const;
    Finger* find(std::uint64_t id);

    std::vector<TouchControl> m_controls;
    std::vector<Finger> m_fingers;
    float m_aspect = 16.0f / 9.0f;
    bool m_visible = false;
    // A finger came down since the last idle(): it counts even if it lifted within the frame.
    bool m_touched = false;
    std::uint64_t m_last_touch_ms = 0;
};

}  // namespace openrac::platform
