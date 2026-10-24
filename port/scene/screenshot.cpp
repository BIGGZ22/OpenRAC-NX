// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "scene/screenshot.h"

#include <stb_image_write.h>

namespace openrac::scene {

bool write_png(
    const std::string& path, int width, int height, const std::vector<std::uint8_t>& rgba
) {
    return stbi_write_png(path.c_str(), width, height, 4, rgba.data(), width * 4) != 0;
}

}  // namespace openrac::scene
