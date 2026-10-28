// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "platform/switch/input.h"
#include "platform/pad.h"
#include <switch.h>
#include <algorithm>

namespace openrac::platform {

int switch_read_pad(int port, std::uint16_t* buttons, std::uint8_t analog[4]) {
    *buttons = 0xFFFF;
    std::fill_n(analog, 4, 0x80);
    if (port != 0) return 0;
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
    const u64 held = padGetButtons(&pad);
    const u64 mapping[16] = {
        HidNpadButton_Minus, HidNpadButton_StickL, HidNpadButton_StickR, HidNpadButton_Plus,
        HidNpadButton_Up, HidNpadButton_Right, HidNpadButton_Down, HidNpadButton_Left,
        HidNpadButton_ZL, HidNpadButton_ZR, HidNpadButton_L, HidNpadButton_R,
        HidNpadButton_X, HidNpadButton_A, HidNpadButton_B, HidNpadButton_Y
    };
    for (unsigned bit = 0; bit < 16; ++bit) {
        if (held & mapping[bit]) *buttons &= static_cast<std::uint16_t>(~(1u << bit));
    }
    const auto axis = [](int value) -> std::int16_t {
        return static_cast<std::int16_t>(std::clamp(value, -32768, 32767));
    };
    const auto right = padGetStickPos(&pad, 1);
    const auto left = padGetStickPos(&pad, 0);
    // Match the DualShock's square gate, including full-speed diagonal movement.
    // Invert Y before narrowing: -(-32768) must saturate, not wrap around.
    stick_bytes(axis(right.x), axis(-right.y), right_gate, analog[0], analog[1]);
    stick_bytes(axis(left.x), axis(-left.y), left_gate, analog[2], analog[3]);
    return 1;
}

}  // namespace openrac::platform
