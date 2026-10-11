// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "platform/input.h"

#include <algorithm>

#include <SDL3/SDL.h>

#include "common/log.h"

namespace openrac::platform {
namespace {

struct KeyBinding {
    SDL_Scancode key;
    Button button;
};

constexpr KeyBinding kKeyButtons[] = {
    {SDL_SCANCODE_UP, Button::Up},
    {SDL_SCANCODE_DOWN, Button::Down},
    {SDL_SCANCODE_LEFT, Button::Left},
    {SDL_SCANCODE_RIGHT, Button::Right},
    {SDL_SCANCODE_SPACE, Button::Cross},
    {SDL_SCANCODE_F, Button::Square},
    {SDL_SCANCODE_E, Button::Circle},
    {SDL_SCANCODE_R, Button::Triangle},
    {SDL_SCANCODE_LSHIFT, Button::L1},
    {SDL_SCANCODE_Q, Button::R1},
    {SDL_SCANCODE_1, Button::L2},
    {SDL_SCANCODE_3, Button::R2},
    {SDL_SCANCODE_C, Button::L3},
    {SDL_SCANCODE_V, Button::R3},
    {SDL_SCANCODE_RETURN, Button::Start},
    {SDL_SCANCODE_BACKSPACE, Button::Select},
};

struct GamepadBinding {
    SDL_GamepadButton pad_button;
    Button button;
};

// By position: SOUTH is Cross on a DualShock and A on an Xbox pad.
constexpr GamepadBinding kGamepadButtons[] = {
    {SDL_GAMEPAD_BUTTON_SOUTH, Button::Cross},
    {SDL_GAMEPAD_BUTTON_EAST, Button::Circle},
    {SDL_GAMEPAD_BUTTON_WEST, Button::Square},
    {SDL_GAMEPAD_BUTTON_NORTH, Button::Triangle},
    {SDL_GAMEPAD_BUTTON_BACK, Button::Select},
    {SDL_GAMEPAD_BUTTON_START, Button::Start},
    {SDL_GAMEPAD_BUTTON_LEFT_STICK, Button::L3},
    {SDL_GAMEPAD_BUTTON_RIGHT_STICK, Button::R3},
    {SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, Button::L1},
    {SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, Button::R1},
    {SDL_GAMEPAD_BUTTON_DPAD_UP, Button::Up},
    {SDL_GAMEPAD_BUTTON_DPAD_DOWN, Button::Down},
    {SDL_GAMEPAD_BUTTON_DPAD_LEFT, Button::Left},
    {SDL_GAMEPAD_BUTTON_DPAD_RIGHT, Button::Right},
};

// A trigger counts as L2 or R2 held above about an eighth of its travel; its
// pressure byte follows the trigger all the way.
constexpr std::uint8_t kTriggerThreshold = 0x20;

bool key(std::span<const bool> keys, SDL_Scancode code) {
    const auto index = static_cast<std::size_t>(code);
    return index < keys.size() && keys[index];
}

// -1, 0 or +1 from two opposite keys.
int key_axis(std::span<const bool> keys, SDL_Scancode negative, SDL_Scancode positive) {
    return (key(keys, positive) ? 1 : 0) - (key(keys, negative) ? 1 : 0);
}

}  // namespace

void apply_keyboard(Pad& pad, std::span<const bool> keys) {
    for (const KeyBinding& binding : kKeyButtons) {
        if (key(keys, binding.key)) {
            pad.set(binding.button, true);
        }
    }
    const int lx = key_axis(keys, SDL_SCANCODE_A, SDL_SCANCODE_D);
    const int ly = key_axis(keys, SDL_SCANCODE_W, SDL_SCANCODE_S);
    const int rx = key_axis(keys, SDL_SCANCODE_J, SDL_SCANCODE_L);
    const int ry = key_axis(keys, SDL_SCANCODE_I, SDL_SCANCODE_K);
    if (lx != 0) {
        pad.left_x = key_stick_byte(lx);
    }
    if (ly != 0) {
        pad.left_y = key_stick_byte(ly);
    }
    if (rx != 0) {
        pad.right_x = key_stick_byte(rx);
    }
    if (ry != 0) {
        pad.right_y = key_stick_byte(ry);
    }
}

Input::~Input() {
    for (SDL_Gamepad* g : m_gamepads) {
        SDL_CloseGamepad(g);
    }
}

void Input::handle_event(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_GAMEPAD_ADDED: {
            SDL_Gamepad* g = SDL_OpenGamepad(event.gdevice.which);
            if (g != nullptr) {
                log::info("gamepad connected: {}", SDL_GetGamepadName(g));
                m_gamepads.push_back(g);
            }
            break;
        }
        case SDL_EVENT_GAMEPAD_REMOVED: {
            auto it = std::find_if(m_gamepads.begin(), m_gamepads.end(), [&](SDL_Gamepad* g) {
                return SDL_GetGamepadID(g) == event.gdevice.which;
            });
            if (it != m_gamepads.end()) {
                log::info("gamepad disconnected");
                SDL_CloseGamepad(*it);
                m_gamepads.erase(it);
            }
            break;
        }
        default:
            break;
    }
}

SDL_Gamepad* Input::gamepad(int port) const {
    const auto index = static_cast<std::size_t>(port);
    return index < m_gamepads.size() ? m_gamepads[index] : nullptr;
}

void Input::update() {
    int key_count = 0;
    const bool* state = SDL_GetKeyboardState(&key_count);
    // The keyboard counts only while the window has it, and not while Cmd/Win is held: a system
    // shortcut (Cmd+Shift+4 takes a screenshot) is not the game's L1, and the key-up of a key held
    // when another window took the keyboard never comes.
    const bool keyboard_ours =
        SDL_GetKeyboardFocus() != nullptr && (SDL_GetModState() & SDL_KMOD_GUI) == 0;
    const std::span<const bool> keys(
        state, state != nullptr && keyboard_ours ? static_cast<std::size_t>(key_count) : 0
    );

    for (int port = 0; port < kPadPorts; ++port) {
        Pad& pad = m_pads[static_cast<std::size_t>(port)];
        // Fresh state each frame, keeping the motors the game asked for.
        const bool small_motor = pad.small_motor;
        const std::uint8_t large_motor = pad.large_motor;
        pad = Pad{};
        pad.small_motor = small_motor;
        pad.large_motor = large_motor;

        SDL_Gamepad* g = gamepad(port);
        pad.connected = port == 0 || g != nullptr;
        if (g != nullptr) {
            for (const GamepadBinding& binding : kGamepadButtons) {
                if (SDL_GetGamepadButton(g, binding.pad_button)) {
                    pad.set(binding.button, true);
                }
            }
            const std::uint8_t l2 =
                trigger_byte(SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_LEFT_TRIGGER));
            const std::uint8_t r2 =
                trigger_byte(SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER));
            if (l2 >= kTriggerThreshold) {
                pad.set(Button::L2, true, l2);
            }
            if (r2 >= kTriggerThreshold) {
                pad.set(Button::R2, true, r2);
            }
            pad.left_x = stick_byte(SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_LEFTX));
            pad.left_y = stick_byte(SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_LEFTY));
            pad.right_x = stick_byte(SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_RIGHTX));
            pad.right_y = stick_byte(SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_RIGHTY));
            // The large motor has a speed, the small one is on or off: they
            // drive SDL's low- and high-frequency rumble. Renewed each frame,
            // so a short duration stops it when the game stops asking.
            const auto low = static_cast<Uint16>(large_motor * 257);
            const auto high = static_cast<Uint16>(small_motor ? 0xFFFF : 0);
            SDL_RumbleGamepad(g, low, high, 100);
        }
        if (port == 0) {
            apply_keyboard(pad, keys);
        }
    }
}

}  // namespace openrac::platform
