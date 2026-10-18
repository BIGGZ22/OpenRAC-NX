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
// on-screen pad (touch_pad.h), also on port 0.

#pragma once

#include <array>
#include <span>
#include <vector>

#include "platform/pad.h"
#include "platform/touch_pad.h"

union SDL_Event;
struct SDL_Gamepad;

namespace openrac::platform {

inline constexpr int kPadPorts = 2;

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

    // The on-screen pad, which adds to port 0 (touch_pad.h). The frontend tells it the window's
    // shape and draws it while it is visible.
    TouchPad& touch_pad() { return m_touch; }

private:
    SDL_Gamepad* gamepad(int port) const;

    std::array<Pad, kPadPorts> m_pads{};
    std::vector<SDL_Gamepad*> m_gamepads;
    TouchPad m_touch;
};

}  // namespace openrac::platform
