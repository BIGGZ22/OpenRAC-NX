// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-video/src/vlc.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The variable-length code tables of ISO/IEC 13818-2 Annex B (the public
// MPEG-2 video standard) that the games' movies use, built once into direct
// lookup tables: one peek of the table's longest code, one lookup. VlcTable is the generic
// table; mpeg2_codes() lists the standard's codes and mpeg2_vlc_tables() has them built.

#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "media/bit_reader.h"

namespace openrac::media {

// One code as the standard prints it ("0000 0101 10"; spaces ignored).
struct VlcCode {
    std::string_view bits;  // The code, most significant bit first.
    std::int16_t value;     // What it decodes to.
};

// A prefix code as a lookup table indexed by the next `max_length` bits.
class VlcTable {
public:
    // Throws std::logic_error when one code is a prefix of another.
    explicit VlcTable(std::span<const VlcCode> codes);

    // Decodes one code; none when the bits match no code (nothing consumed).
    std::optional<std::int16_t> decode(BitReader& reader) const {
        const Entry e = m_table[reader.peek(m_bits)];

        // No code starts with these bits: nothing is consumed.
        if (e.length == 0) {
            return std::nullopt;
        }

        reader.skip(e.length);
        return e.value;
    }

    // The number of bits one lookup peeks: the length of the longest code.
    unsigned max_length() const { return m_bits; }

    // The sum of 2^-length over the codes (1 for a complete code).
    double kraft() const;

private:
    struct Entry {
        std::int16_t value = 0;
        std::uint8_t length = 0;  // 0: no code starts with these bits
    };

    // The length of the longest code, which is also the width of the table's index.
    unsigned m_bits = 1;

    // One entry for each value of the next `m_bits` bits; a code fills every entry it prefixes.
    std::vector<Entry> m_table;
};

// macroblock_address_increment's escape (+33) and MPEG-1 stuffing.
inline constexpr std::int16_t kMbaEscape = -1;
inline constexpr std::int16_t kMbaStuffing = -2;

// macroblock_type flags.
inline constexpr std::int16_t kMbQuant = 1;
inline constexpr std::int16_t kMbForward = 2;
inline constexpr std::int16_t kMbBackward = 4;
inline constexpr std::int16_t kMbPattern = 8;
inline constexpr std::int16_t kMbIntra = 16;

// DCT coefficient codes are `run << 8 | level` (the level without its sign
// bit), or one of these.
inline constexpr std::int16_t kDctEndOfBlock = -1;
inline constexpr std::int16_t kDctEscape = -2;

// The code lists, as the standard's tables.
enum class Mpeg2Codes {
    MacroblockAddress,  // B-1
    MacroblockTypeI,    // B-2
    MacroblockTypeP,    // B-3
    MacroblockTypeB,    // B-4
    CodedBlockPattern,  // B-9 (4:2:0)
    MotionCode,         // B-10, magnitudes; the sign bit follows
    DcSizeLuminance,    // B-12
    DcSizeChrominance,  // B-13
    DctZero,            // B-14 without the long codes
    DctOne,             // B-15 without the long codes
    DctLong,            // the 14..16-bit codes common to B-14 and B-15
};

// The codes of one table of the standard, in the order the standard lists them.
std::span<const VlcCode> mpeg2_codes(Mpeg2Codes which);

// The tables the decoder uses, built from the codes above.
struct Mpeg2VlcTables {
    VlcTable macroblock_address;
    VlcTable macroblock_type[3];  // I, P, B
    VlcTable coded_block_pattern;
    VlcTable motion_code;
    VlcTable dc_size_luminance;
    VlcTable dc_size_chrominance;
    VlcTable dct_zero;  // B-14 with the long codes
    VlcTable dct_one;   // B-15 with the long codes
};

// Built on first use.
const Mpeg2VlcTables& mpeg2_vlc_tables();

}  // namespace openrac::media
