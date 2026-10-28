// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string_view>

namespace openrac::platform::controller {
// Stable file order, independent of libnx's bit numbering.
inline constexpr std::array<const char*, 16> kPhysicalNames =
    {"A",
     "B",
     "X",
     "Y",
     "L",
     "R",
     "ZL",
     "ZR",
     "+",
     "-",
     "L Stick",
     "R Stick",
     "Up",
     "Right",
     "Down",
     "Left"};
inline constexpr std::array<const char*, 16> kPhysicalKeys =
    {"a",
     "b",
     "x",
     "y",
     "l",
     "r",
     "zl",
     "zr",
     "plus",
     "minus",
     "l3",
     "r3",
     "up",
     "right",
     "down",
     "left"};
// Game actions are in the DualShock's active-low packet order.
inline constexpr std::array<const char*, 16> kActionKeys =
    {"select",
     "l3",
     "r3",
     "start",
     "up",
     "right",
     "down",
     "left",
     "l2",
     "r2",
     "l1",
     "r1",
     "triangle",
     "circle",
     "cross",
     "square"};
inline constexpr std::array<const char*, 16> kActionNames =
    {"Map / Select",
     "L3",
     "R3",
     "Pause / Start",
     "D-pad Up",
     "D-pad Right",
     "D-pad Down",
     "D-pad Left",
     "Aim / lock-strafe (L2)",
     "R2",
     "L1",
     "R1",
     "Triangle",
     "Shoot / Circle",
     "Jump / Cross",
     "Attack / Square"};

struct Profile {
    std::array<int, 16> buttons = {9, 10, 11, 8, 12, 13, 14, 15, 6, 7, 4, 5, 2, 0, 1, 3};
    int movement_deadzone = 0, camera_deadzone = 0;
    int camera_x = 100, camera_y = 100;
    bool invert_x = false, invert_y = false, swap_sticks = false;
};
enum class CaptureState {
    ReleaseButtons,
    PressButton,
    ReleaseToConfirm,
    MultipleButtons,
    Bound,
    Cancelled
};

struct BindingCapture {
    bool armed = false;
    int physical = -1;
    CaptureState state = CaptureState::ReleaseButtons;

    CaptureState update(std::uint16_t held) {
        if (state == CaptureState::Bound || state == CaptureState::Cancelled) {
            return state;
        }
        if ((held & ((1u << 8) | (1u << 9))) == ((1u << 8) | (1u << 9))) {
            return state = CaptureState::Cancelled;
        }
        if (!armed) {
            if (!held) {
                armed = true;
                state = CaptureState::PressButton;
            }
        } else if (physical < 0) {
            if (held && (held & (held - 1))) {
                armed = false;
                state = CaptureState::MultipleButtons;
            } else if (held) {
                for (int i = 0; i < 16; ++i) {
                    if (held & (1u << i)) {
                        physical = i;
                    }
                }
                state = CaptureState::ReleaseToConfirm;
            }
        } else if (!held) {
            state = CaptureState::Bound;
        } else if (held != (1u << physical)) {
            armed = false;
            physical = -1;
            state = CaptureState::MultipleButtons;
        }
        return state;
    }
};

inline int physical_from_key(std::string_view name) {
    if (name == "unbound") {
        return -1;
    }
    for (unsigned i = 0; i < kPhysicalKeys.size(); ++i) {
        if (name == kPhysicalKeys[i]) {
            return int(i);
        }
    }
    return -2;
}

inline const char* binding_name(int physical) {
    return physical >= 0 && physical < 16 ? kPhysicalNames[physical] : "Unbound";
}

inline const char* binding_key(int physical) {
    return physical >= 0 && physical < 16 ? kPhysicalKeys[physical] : "unbound";
}

inline void bind(Profile& profile, unsigned action, int physical) {
    if (action >= 16 || physical < -1 || physical >= 16) {
        return;
    }
    const int previous = profile.buttons[action];
    bool swapped = false;
    if (physical >= 0) {
        for (unsigned i = 0; i < 16; ++i) {
            if (i != action && profile.buttons[i] == physical) {
                profile.buttons[i] = swapped ? -1 : previous;
                swapped = true;
            }
        }
    }
    profile.buttons[action] = physical;
}

inline void sanitize(Profile& p) {
    p.movement_deadzone = std::clamp(p.movement_deadzone, 0, 30);
    p.camera_deadzone = std::clamp(p.camera_deadzone, 0, 30);
    p.camera_x = std::clamp(p.camera_x, 25, 200);
    p.camera_y = std::clamp(p.camera_y, 25, 200);
    unsigned seen = 0;
    for (auto& b : p.buttons) {
        if (b < -1 || b >= 16 || (b >= 0 && (seen & (1u << b)))) {
            b = -1;
        }
        if (b >= 0) {
            seen |= 1u << b;
        }
    }
}

inline std::uint16_t buttons(const Profile& p, std::uint16_t physical_held) {
    std::uint16_t result = 0xffff;
    for (unsigned action = 0; action < 16; ++action) {
        const int physical = p.buttons[action];
        if (physical >= 0 && physical < 16 && (physical_held & (1u << physical))) {
            result &= std::uint16_t(~(1u << action));
        }
    }
    return result;
}

inline void deadzone(float& x, float& y, int percent) {
    const float zone = static_cast<float>(std::clamp(percent, 0, 30)) / 100.0f;
    if (zone == 0) {
        return;
    }
    const float length = std::sqrt(x * x + y * y);
    if (length <= zone) {
        x = y = 0;
        return;
    }
    const float scale = (std::min(1.0f, length) - zone) / ((1 - zone) * length);
    x *= scale;
    y *= scale;
}

// Input and output are left X/Y, right X/Y in the game's axis directions.
// Gate shaping happens afterwards, keeping full-speed movement at diagonals.
inline std::array<float, 4> sticks(const Profile& p, std::array<float, 4> axes) {
    if (p.swap_sticks) {
        std::swap(axes[0], axes[2]);
        std::swap(axes[1], axes[3]);
    }
    deadzone(axes[0], axes[1], p.movement_deadzone);
    deadzone(axes[2], axes[3], p.camera_deadzone);
    axes[2] *=
        (p.invert_x ? -1.0f : 1.0f) * static_cast<float>(std::clamp(p.camera_x, 25, 200)) / 100.0f;
    axes[3] *=
        (p.invert_y ? -1.0f : 1.0f) * static_cast<float>(std::clamp(p.camera_y, 25, 200)) / 100.0f;
    for (auto& a : axes) {
        a = std::clamp(a, -1.0f, 1.0f);
    }
    return axes;
}
}  // namespace openrac::platform::controller
