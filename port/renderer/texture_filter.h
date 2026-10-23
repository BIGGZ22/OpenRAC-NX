// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#pragma once
#include <cstdint>

namespace openrac::renderer {

inline constexpr unsigned kUpscaledTextureColoursUnit = 13;
enum class TextureFilter {
    Normal,
    EdgeAware,
    Count
};

// Optional provider of upscaled texture colours. The source remains authoritative
// for alpha and minification. GPU objects are released while the context is current.
class UpscaledTextureColours {
public:
    virtual ~UpscaledTextureColours() = default;
    virtual unsigned prepare(
        unsigned source, int width, int height, unsigned wrap, std::uint64_t frame
    ) = 0;
    virtual void forget(unsigned source) = 0;
    virtual void release() = 0;
};
}  // namespace openrac::renderer
