// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "assets/image.h"

#include <array>

namespace openrac::assets {

RgbaImage decode_indexed8(ByteView indices, u32 width, u32 height, ByteView clut, GsAlpha alpha) {
    const std::size_t n = std::size_t{width} * height;

    if (indices.size() < n) {
        fail("indexed image: {} pixels need {} bytes, have {}", n, n, indices.size());
    }

    if (clut.size() < 1024) {
        fail("indexed image: a palette of {} bytes, need 1024", clut.size());
    }

    std::array<std::array<u8, 4>, 256> palette{};

    for (u32 i = 0; i < 256; ++i) {
        const u8* e = clut.data() + clut_index(i) * 4;
        palette[i] = {e[0], e[1], e[2], alpha == GsAlpha::Scaled ? scale_alpha(e[3]) : e[3]};
    }

    RgbaImage image{width, height, {}};
    image.rgba.reserve(n * 4);

    for (std::size_t p = 0; p < n; ++p) {
        const auto& c = palette[indices.data()[p]];
        image.rgba.insert(image.rgba.end(), c.begin(), c.end());
    }

    return image;
}

}  // namespace openrac::assets
