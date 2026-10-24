// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Writing a frame to a PNG, for the viewer's headless checks.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace openrac::viewer {

// `rgba` is width x height RGBA bytes, top row first.
bool write_png(
    const std::string& path, int width, int height, const std::vector<std::uint8_t>& rgba
);

}  // namespace openrac::viewer
