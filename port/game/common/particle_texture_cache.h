// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#pragma once

#include <cstdint>
#include <vector>

namespace openrac::game {

// Use only within one particle extraction pass, while game RAM is immutable.
// Addresses are not content identities: never retain this cache between passes.
class ParticleTextureCache {
public:
    template <class Resolve>
    std::uint32_t resolve(std::uint32_t pixels, std::uint32_t palette, Resolve&& uncached) {
        const std::uint64_t address = (std::uint64_t(palette) << 32) | pixels;
        for (const Entry& entry : entries_) {
            if (entry.address == address) {
                return entry.texture;
            }
        }
        const std::uint32_t texture = uncached(pixels, palette);
        entries_.push_back({address, texture});
        return texture;
    }

private:
    struct Entry {
        std::uint64_t address;
        std::uint32_t texture;
    };

    std::vector<Entry> entries_;
};

}  // namespace openrac::game
