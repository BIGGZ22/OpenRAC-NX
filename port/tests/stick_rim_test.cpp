// SPDX-License-Identifier: GPL-3.0-or-later
#include "platform/pad.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

int main() {
    using namespace openrac::platform;
    StickGate gate;
    for (int angle = 0; angle < 360; ++angle) {
        const double radians = angle * 3.141592653589793 / 180.0;
        const auto input = [](double value) {
            return static_cast<std::int16_t>(std::clamp(std::lround(value), -32768L, 32767L));
        };
        std::uint8_t x, y;
        stick_bytes(input(std::cos(radians) * 32767), input(std::sin(radians) * 32767), gate, x, y);
        // Every full-rim direction must reach the console's outer range.
        if (std::max(std::abs(static_cast<int>(x) - 128), std::abs(static_cast<int>(y) - 128)) < 126) {
            std::fprintf(stderr, "Full tilt reduced at %d degrees: %u %u\n", angle, x, y);
            return 1;
        }
    }
    std::uint8_t x, y;
    stick_bytes(0, 0, gate, x, y);
    if (x != 128 || y != 128) return 2;
    stick_bytes(11585, 11585, gate, x, y);
    if (x != stick_byte(11585) || y != stick_byte(11585)) return 3;
    return 0;
}
