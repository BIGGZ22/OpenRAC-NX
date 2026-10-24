// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "platform/pad.h"

#include <algorithm>
#include <cmath>

namespace openrac::platform {

Pressure pressure_of(Button button) {
    switch (button) {
        case Button::Right:
            return Pressure::Right;
        case Button::Left:
            return Pressure::Left;
        case Button::Up:
            return Pressure::Up;
        case Button::Down:
            return Pressure::Down;
        case Button::Triangle:
            return Pressure::Triangle;
        case Button::Circle:
            return Pressure::Circle;
        case Button::Cross:
            return Pressure::Cross;
        case Button::Square:
            return Pressure::Square;
        case Button::L1:
            return Pressure::L1;
        case Button::R1:
            return Pressure::R1;
        case Button::L2:
            return Pressure::L2;
        case Button::R2:
            return Pressure::R2;
        case Button::Select:
        case Button::L3:
        case Button::R3:
        case Button::Start:
            break;
    }
    return Pressure::Count;
}

void Pad::set(Button button, bool down, std::uint8_t amount) {
    const auto bit = static_cast<std::uint16_t>(button);
    if (down) {
        buttons = static_cast<std::uint16_t>(buttons & ~bit);
    } else {
        buttons = static_cast<std::uint16_t>(buttons | bit);
    }
    const Pressure p = pressure_of(button);
    if (p != Pressure::Count) {
        pressure[static_cast<std::size_t>(p)] = down ? amount : 0;
    }
}

void Pad::write_read_buffer(std::span<std::uint8_t, kPadBufferSize> out) const {
    std::fill(out.begin(), out.end(), std::uint8_t{0});
    if (!connected) {
        out[0] = 0xFF;
        return;
    }
    out[0] = 0x00;
    out[1] = kModePressure;
    out[2] = static_cast<std::uint8_t>(buttons & 0xFF);
    out[3] = static_cast<std::uint8_t>(buttons >> 8);
    out[4] = right_x;
    out[5] = right_y;
    out[6] = left_x;
    out[7] = left_y;
    std::copy(pressure.begin(), pressure.end(), out.begin() + 8);
}

std::uint8_t stick_byte(std::int16_t axis) {
    // -32768..32767 shifted to 0..65535, keeping the top byte: 0 lands on
    // 0x80 exactly, the extremes on 0x00 and 0xFF.
    return static_cast<std::uint8_t>((static_cast<int>(axis) + 32768) >> 8);
}

void square_stick(float& x, float& y) {
    const float larger = std::max(std::abs(x), std::abs(y));
    const float length = std::sqrt(x * x + y * y);
    if (larger <= 0.0f) {
        return;
    }
    // From 0.8 of the rim out, eased in, to the square on the rim.
    const float t = std::clamp((length - 0.8f) / 0.2f, 0.0f, 1.0f);
    const float rim = t * t * (3.0f - 2.0f * t);
    if (rim <= 0.0f) {
        return;
    }
    const float stretched = larger + (std::min(length, 1.0f) - larger) * rim;
    const float scale = stretched / larger;
    x = std::clamp(x * scale, -1.0f, 1.0f);
    y = std::clamp(y * scale, -1.0f, 1.0f);
}

void shape_stick(float& x, float& y, StickGate& gate) {
    // A round gate, even one a little out of true, stays well within 1.25; a square one's corner
    // is near 1.41.
    if (x * x + y * y > 1.25f * 1.25f) {
        gate.square = true;
    }
    if (!gate.square) {
        square_stick(x, y);
    }
}

void stick_bytes(
    std::int16_t x, std::int16_t y, StickGate& gate, std::uint8_t& out_x, std::uint8_t& out_y
) {
    float fx = static_cast<float>(x) / 32768.0f;
    float fy = static_cast<float>(y) / 32768.0f;
    shape_stick(fx, fy, gate);
    const auto axis = [](float v) {
        return static_cast<std::int16_t>(std::clamp(std::lround(v * 32768.0f), -32768L, 32767L));
    };
    out_x = stick_byte(axis(fx));
    out_y = stick_byte(axis(fy));
}

std::uint8_t trigger_byte(std::int16_t axis) {
    const int value = std::clamp(static_cast<int>(axis), 0, 32767);
    return static_cast<std::uint8_t>((value * 255 + 16383) / 32767);
}

std::uint8_t key_stick_byte(int direction) {
    if (direction < 0) {
        return 0x00;
    }
    if (direction > 0) {
        return 0xFF;
    }
    return kStickCentre;
}

}  // namespace openrac::platform
