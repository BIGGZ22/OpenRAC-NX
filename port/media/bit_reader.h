// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-video/src/bits.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// A big-endian bit reader over an MPEG video elementary stream, and the start-code search
// (ISO/IEC 13818-2: `00 00 01 xx`).
//
// The reader keeps one bit position into the stream. A read gathers the eight bytes that hold
// the position into a 64-bit word and shifts out the bits already used, so any read of up to 32
// bits is one shift of that word. Reading past the end gives zero bits and still moves the
// position on, and overrun() reports it, so a caller can read freely and check afterwards.

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace openrac::media {

// Reads bits most significant first. Reading past the end yields zero bits;
// callers check overrun().
class BitReader {
public:
    // Starts reading `data` at byte `byte_pos`.
    BitReader(std::span<const std::uint8_t> data, std::size_t byte_pos)
        : m_data(data),
          m_pos(byte_pos * 8) {}

    // The position in bits from the start of the stream.
    std::size_t bit_pos() const { return m_pos; }

    // The byte the position is in.
    std::size_t byte_pos() const { return m_pos >> 3; }

    // True once a read has gone past the end of the data.
    bool overrun() const { return m_pos > m_data.size() * 8; }

    // The next `n` bits (1..32) without consuming them.
    std::uint32_t peek(unsigned n) const;

    // Consumes `n` bits without reading them.
    void skip(unsigned n) { m_pos += n; }

    // Reads the next `n` bits (1..32) and consumes them.
    std::uint32_t read(unsigned n) {
        const std::uint32_t v = peek(n);
        m_pos += n;
        return v;
    }

    // Reads one bit.
    bool bit() { return read(1) != 0; }

    // To the next byte boundary.
    void align() { m_pos = (m_pos + 7) & ~std::size_t{7}; }

    // The next 23 bits are zero: a start code follows (the end of a slice).
    bool at_start_code() const { return peek(23) == 0; }

private:
    // The eight bytes from the current byte on as one big-endian word; bytes past the end are zero.
    // Eight bytes are enough: a 32-bit read that starts up to 7 bits into a byte needs 39 bits.
    std::uint64_t window() const;

    // The stream; not owned, and it outlives the reader.
    std::span<const std::uint8_t> m_data;

    // The position in bits.
    std::size_t m_pos;
};

// The offset of the next `00 00 01` at or after `from`, or none.
std::optional<std::size_t> next_start_code(std::span<const std::uint8_t> data, std::size_t from);

}  // namespace openrac::media
