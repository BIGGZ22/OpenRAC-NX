// SPDX-License-Identifier: GPL-3.0-or-later
#include "platform/pad.h"

#include <algorithm>
#include <cmath>
#include <switch.h>

#include "platform/switch/controller_profile.h"
#include "platform/switch/input.h"
#ifdef OPENRAC_FRONTEND
#include "settings.h"
#endif
namespace openrac::platform {
std::uint16_t switch_physical_buttons(std::uint64_t held) {
    const std::uint64_t mapping[16] =
        {HidNpadButton_A,
         HidNpadButton_B,
         HidNpadButton_X,
         HidNpadButton_Y,
         HidNpadButton_L,
         HidNpadButton_R,
         HidNpadButton_ZL,
         HidNpadButton_ZR,
         HidNpadButton_Plus,
         HidNpadButton_Minus,
         HidNpadButton_StickL,
         HidNpadButton_StickR,
         HidNpadButton_Up,
         HidNpadButton_Right,
         HidNpadButton_Down,
         HidNpadButton_Left};
    std::uint16_t result = 0;
    for (unsigned i = 0; i < 16; ++i) {
        if (held & mapping[i]) {
            result |= std::uint16_t(1u << i);
        }
    }
    return result;
}

int switch_read_pad(int port, std::uint16_t* buttons, std::uint8_t analog[4]) {
    *buttons = 0xffff;
    std::fill_n(analog, 4, 0x80);
    if (port != 0) {
        return 0;
    }
    static PadState pad;
    static StickGate left_gate, right_gate;
    static const bool initialized = [] {
        padConfigureInput(1, HidNpadStyleSet_NpadStandard);
        padInitializeDefault(&pad);
        return true;
    }();
    (void)initialized;
    padUpdate(&pad);
    if (!padIsConnected(&pad)) {
        left_gate = {};
        right_gate = {};
        return 0;
    }
#ifdef OPENRAC_FRONTEND
    const auto& profile = settings::get().controller;
#else
    const controller::Profile profile;
#endif
    *buttons = controller::buttons(profile, switch_physical_buttons(padGetButtons(&pad)));
    const auto left = padGetStickPos(&pad, 0), right = padGetStickPos(&pad, 1);
    const auto axes = controller::sticks(
        profile, {left.x / 32768.0f, -left.y / 32768.0f, right.x / 32768.0f, -right.y / 32768.0f}
    );
    const auto axis = [](float value) {
        return static_cast<std::int16_t>(
            std::clamp(std::lround(value * 32768.0f), -32768L, 32767L)
        );
    };
    // Keep the existing circular-to-square gate correction after deadzone and
    // camera adjustments, so full tilt still runs in every movement direction.
    stick_bytes(axis(axes[2]), axis(axes[3]), right_gate, analog[0], analog[1]);
    stick_bytes(axis(axes[0]), axis(axes[1]), left_gate, analog[2], analog[3]);
    return 1;
}
}  // namespace openrac::platform
