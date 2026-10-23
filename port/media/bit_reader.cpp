// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-video/src/bits.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The bit reader (bit_reader.h).

#include "media/bit_reader.h"

namespace openrac::media {

std::uint64_t BitReader::window() const {
    const std::size_t b = m_pos >> 3;
    std::uint64_t w = 0;

    if (b + 8 <= m_data.size()) {
        for (std::size_t k = 0; k < 8; ++k) {
            w = w << 8 | m_data[b + k];
        }

        return w;
    }

    for (std::size_t k = 0; k < 8; ++k) {
        const std::uint64_t byte = b + k < m_data.size() ? m_data[b + k] : 0;
        w = w << 8 | byte;
    }

    return w;
}

std::uint32_t BitReader::peek(unsigned n) const {
    return static_cast<std::uint32_t>((window() << (m_pos & 7)) >> (64 - n));
}

std::optional<std::size_t> next_start_code(std::span<const std::uint8_t> data, std::size_t from) {
    std::size_t i = from;

    while (i + 3 <= data.size()) {
        // Skip fast over bytes that cannot end a start code prefix.
        if (data[i + 2] > 1) {
            i += 3;
        } else if (data[i + 2] == 1 && data[i] == 0 && data[i + 1] == 0) {
            return i;
        } else {
            ++i;
        }
    }

    return std::nullopt;
}

}  // namespace openrac::media
