// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>

namespace openrac::platform {
std::uint16_t switch_physical_buttons(std::uint64_t held);
int switch_read_pad(int port, std::uint16_t* buttons, std::uint8_t analog[4]);
}  // namespace openrac::platform
