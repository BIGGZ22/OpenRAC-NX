// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The keyboard and SDL's gamepads, turned into the two DualShock 2 ports the
// game reads (pad.h). Port 0 is the keyboard together with the first gamepad;
// port 1 is the second gamepad. Gamepad buttons are mapped by position, so the
// bottom face button is Cross on any controller.
//
// Default keyboard (by key position, whatever the layout):
//
//   left stick  W A S D        right stick  I J K L
//   d-pad       arrow keys     Cross Space, Square F, Circle E, Triangle R
//   L1 Left Shift, R1 Q, L2 1, R2 3, L3 C, R3 V, Start Return, Select Backspace
//
// Remapping comes with the settings file; the table is in input.cpp. A touch screen gets an
// on-screen pad (touch_pad.h), also on port 0. A mod may take the keyboard (mod interface 1.2,
// openrac_api.take_input): the table then stops, and the mod presses pad 0's buttons itself from
// the keyboard and mouse state below (KeyboardMouse), which is read for it every frame.

#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "platform/pad.h"
#include "platform/touch_pad.h"

union SDL_Event;
struct SDL_Gamepad;

namespace openrac::platform {

inline constexpr int kPadPorts = 2;
inline constexpr std::size_t kKeyCount = 512;  // SDL scancodes, which are USB HID usages

// The keyboard and mouse for the tick about to run, as a mod reads them (openrac_mod.h's
// openrac_input has the same meaning, field by field).
struct KeyboardMouse {
    std::uint32_t tick = 0;
    std::array<std::uint8_t, kKeyCount> keys{};  // bit 0 held, bit 1 went down since the last tick
    std::uint32_t mouse_held = 0;                // SDL_BUTTON_*MASK: left 1, middle 2, right 4, x1 8, x2 16
    std::uint32_t mouse_pressed = 0;
    float mouse_dx = 0.0f, mouse_dy = 0.0f;
    float wheel_x = 0.0f, wheel_y = 0.0f;  // notches; y > 0 away from the player
    bool focused = false;
    bool captured = false;
};

// Apply the keyboard table to a pad: `keys` is indexed by SDL scancode, as
// SDL_GetKeyboardState returns it. A stick key overrides the stick's axis.
void apply_keyboard(Pad& pad, std::span<const bool> keys);

class Input {
public:
    Input() = default;
    ~Input();
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    // Every SDL event goes through here (gamepads arriving and leaving).
    void handle_event(const SDL_Event& event);

    // Read the keyboard and gamepads into the pads; once per game frame,
    // after the events.
    void update();

    const Pad& pad(int port) const { return m_pads[static_cast<std::size_t>(port)]; }

    // The game sets the motors here (scePadSetActDirect); update() plays them.
    Pad& pad(int port) { return m_pads[static_cast<std::size_t>(port)]; }

    // The keyboard and mouse as update() last read them, for the mods.
    const KeyboardMouse& keyboard_mouse() const { return m_km; }

    // False: the keyboard table no longer presses pad 0's buttons (a mod took the keyboard).
    void set_keyboard_table(bool on) { m_keyboard_table = on; }

    // Whether the cursor is captured now (the frontend sets it: SDL's relative mouse mode).
    void set_captured(bool captured) { m_km.captured = captured; }

    // The on-screen pad, which adds to port 0 (touch_pad.h). The frontend tells it the window's
    // shape and draws it while it is visible.
    TouchPad& touch_pad() { return m_touch; }

private:
    SDL_Gamepad* gamepad(int port) const;

    std::array<Pad, kPadPorts> m_pads{};
    std::vector<SDL_Gamepad*> m_gamepads;
    TouchPad m_touch;
    KeyboardMouse m_km;
    bool m_keyboard_table = true;
    // Gathered from the events between two update()s.
    std::array<std::uint8_t, kKeyCount> m_went_down{};
    std::uint32_t m_mouse_went_down = 0;
    float m_dx = 0.0f, m_dy = 0.0f, m_wheel_x = 0.0f, m_wheel_y = 0.0f;
};

}  // namespace openrac::platform
