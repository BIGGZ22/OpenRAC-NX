// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
namespace openrac::platform {
int switch_read_pad(int port, std::uint16_t* buttons, std::uint8_t analog[4]);
}
