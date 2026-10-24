// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-video/src/vlc.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The MPEG-2 code tables (vlc.h). The codes are ISO/IEC 13818-2 Annex B's, as the standard
// prints them, with the spaces it uses to group the bits.
//
// The file has three parts: the tables as data, the constructor that turns a list of codes into
// a lookup table, and the accessors that hand the tables out. A table is checked as it is built:
// a typo in a code, or two codes where one is a prefix of the other, throws.

#include "media/vlc.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace openrac::media {

namespace {

// The longest code in the standard's tables is 16 bits (the long DCT codes of B-14 and B-15).
constexpr unsigned kMaxCodeBits = 16;

// A run/level pair as a table value: the run in the high byte, the level in the low byte.
constexpr std::int16_t rl(int run, int level) {
    return static_cast<std::int16_t>(run << 8 | level);
}

// ---- Code tables ----

// The tables are in the order of the enumeration in vlc.h. B-14 below is table zero without the
// sign bits and without the long codes; "11" is (0, 1) except as the first coefficient of a
// non-intra block, where "1" is (the decoder handles that case). B-15 is table one (intra blocks
// with intra_vlc_format = 1).

// Table B-1: macroblock_address_increment.
constexpr VlcCode kMacroblockAddress[] = {
    {"1", 1},
    {"011", 2},
    {"010", 3},
    {"0011", 4},
    {"0010", 5},
    {"0001 1", 6},
    {"0001 0", 7},
    {"0000 111", 8},
    {"0000 110", 9},
    {"0000 1011", 10},
    {"0000 1010", 11},
    {"0000 1001", 12},
    {"0000 1000", 13},
    {"0000 0111", 14},
    {"0000 0110", 15},
    {"0000 0101 11", 16},
    {"0000 0101 10", 17},
    {"0000 0101 01", 18},
    {"0000 0101 00", 19},
    {"0000 0100 11", 20},
    {"0000 0100 10", 21},
    {"0000 0100 011", 22},
    {"0000 0100 010", 23},
    {"0000 0100 001", 24},
    {"0000 0100 000", 25},
    {"0000 0011 111", 26},
    {"0000 0011 110", 27},
    {"0000 0011 101", 28},
    {"0000 0011 100", 29},
    {"0000 0011 011", 30},
    {"0000 0011 010", 31},
    {"0000 0011 001", 32},
    {"0000 0011 000", 33},
    {"0000 0001 000", kMbaEscape},
    {"0000 0001 111", kMbaStuffing},
};

// Table B-2: macroblock_type in an I picture.
constexpr VlcCode kMacroblockTypeI[] = {
    {"1", kMbIntra},
    {"01", kMbIntra | kMbQuant},
};

// Table B-3: macroblock_type in a P picture.
constexpr VlcCode kMacroblockTypeP[] = {
    {"1", kMbForward | kMbPattern},
    {"01", kMbPattern},
    {"001", kMbForward},
    {"0001 1", kMbIntra},
    {"0001 0", kMbQuant | kMbForward | kMbPattern},
    {"0000 1", kMbQuant | kMbPattern},
    {"0000 01", kMbQuant | kMbIntra},
};

// Table B-4: macroblock_type in a B picture.
constexpr VlcCode kMacroblockTypeB[] = {
    {"10", kMbForward | kMbBackward},
    {"11", kMbForward | kMbBackward | kMbPattern},
    {"010", kMbBackward},
    {"011", kMbBackward | kMbPattern},
    {"0010", kMbForward},
    {"0011", kMbForward | kMbPattern},
    {"0001 1", kMbIntra},
    {"0001 0", kMbQuant | kMbForward | kMbBackward | kMbPattern},
    {"0000 11", kMbQuant | kMbForward | kMbPattern},
    {"0000 10", kMbQuant | kMbBackward | kMbPattern},
    {"0000 01", kMbQuant | kMbIntra},
};

// Table B-9: coded_block_pattern for 4:2:0.
constexpr VlcCode kCodedBlockPattern[] = {
    {"111", 60},         {"1101", 4},         {"1100", 8},         {"1011", 16},
    {"1010", 32},        {"1001 1", 12},      {"1001 0", 48},      {"1000 1", 20},
    {"1000 0", 40},      {"0111 1", 28},      {"0111 0", 44},      {"0110 1", 52},
    {"0110 0", 56},      {"0101 1", 1},       {"0101 0", 61},      {"0100 1", 2},
    {"0100 0", 62},      {"0011 11", 24},     {"0011 10", 36},     {"0011 01", 3},
    {"0011 00", 63},     {"0010 111", 5},     {"0010 110", 9},     {"0010 101", 17},
    {"0010 100", 33},    {"0010 011", 6},     {"0010 010", 10},    {"0010 001", 18},
    {"0010 000", 34},    {"0001 1111", 7},    {"0001 1110", 11},   {"0001 1101", 19},
    {"0001 1100", 35},   {"0001 1011", 13},   {"0001 1010", 49},   {"0001 1001", 21},
    {"0001 1000", 41},   {"0001 0111", 14},   {"0001 0110", 50},   {"0001 0101", 22},
    {"0001 0100", 42},   {"0001 0011", 15},   {"0001 0010", 51},   {"0001 0001", 23},
    {"0001 0000", 43},   {"0000 1111", 25},   {"0000 1110", 37},   {"0000 1101", 26},
    {"0000 1100", 38},   {"0000 1011", 29},   {"0000 1010", 45},   {"0000 1001", 53},
    {"0000 1000", 57},   {"0000 0111", 30},   {"0000 0110", 46},   {"0000 0101", 54},
    {"0000 0100", 58},   {"0000 0011 1", 31}, {"0000 0011 0", 47}, {"0000 0010 1", 55},
    {"0000 0010 0", 59}, {"0000 0001 1", 27}, {"0000 0001 0", 39}, {"0000 0000 1", 0},
};

// Table B-10: the magnitude of motion_code; the sign bit follows it.
constexpr VlcCode kMotionCode[] = {
    {"1", 0},
    {"01", 1},
    {"001", 2},
    {"0001", 3},
    {"0000 11", 4},
    {"0000 101", 5},
    {"0000 100", 6},
    {"0000 011", 7},
    {"0000 0101 1", 8},
    {"0000 0101 0", 9},
    {"0000 0100 1", 10},
    {"0000 0100 01", 11},
    {"0000 0100 00", 12},
    {"0000 0011 11", 13},
    {"0000 0011 10", 14},
    {"0000 0011 01", 15},
    {"0000 0011 00", 16},
};

// Table B-12: dct_dc_size_luminance.
constexpr VlcCode kDcSizeLuminance[] = {
    {"100", 0},
    {"00", 1},
    {"01", 2},
    {"101", 3},
    {"110", 4},
    {"1110", 5},
    {"1111 0", 6},
    {"1111 10", 7},
    {"1111 110", 8},
    {"1111 1110", 9},
    {"1111 1111 0", 10},
    {"1111 1111 1", 11},
};

// Table B-13: dct_dc_size_chrominance.
constexpr VlcCode kDcSizeChrominance[] = {
    {"00", 0},
    {"01", 1},
    {"10", 2},
    {"110", 3},
    {"1110", 4},
    {"1111 0", 5},
    {"1111 10", 6},
    {"1111 110", 7},
    {"1111 1110", 8},
    {"1111 1111 0", 9},
    {"1111 1111 10", 10},
    {"1111 1111 11", 11},
};

// The codes of 14 to 16 bits that B-14 and B-15 have in common.
constexpr VlcCode kDctLong[] = {
    {"0000 0000 0111 11", rl(0, 16)},   {"0000 0000 0111 10", rl(0, 17)},
    {"0000 0000 0111 01", rl(0, 18)},   {"0000 0000 0111 00", rl(0, 19)},
    {"0000 0000 0110 11", rl(0, 20)},   {"0000 0000 0110 10", rl(0, 21)},
    {"0000 0000 0110 01", rl(0, 22)},   {"0000 0000 0110 00", rl(0, 23)},
    {"0000 0000 0101 11", rl(0, 24)},   {"0000 0000 0101 10", rl(0, 25)},
    {"0000 0000 0101 01", rl(0, 26)},   {"0000 0000 0101 00", rl(0, 27)},
    {"0000 0000 0100 11", rl(0, 28)},   {"0000 0000 0100 10", rl(0, 29)},
    {"0000 0000 0100 01", rl(0, 30)},   {"0000 0000 0100 00", rl(0, 31)},
    {"0000 0000 0011 000", rl(0, 32)},  {"0000 0000 0010 111", rl(0, 33)},
    {"0000 0000 0010 110", rl(0, 34)},  {"0000 0000 0010 101", rl(0, 35)},
    {"0000 0000 0010 100", rl(0, 36)},  {"0000 0000 0010 011", rl(0, 37)},
    {"0000 0000 0010 010", rl(0, 38)},  {"0000 0000 0010 001", rl(0, 39)},
    {"0000 0000 0010 000", rl(0, 40)},  {"0000 0000 0011 111", rl(1, 8)},
    {"0000 0000 0011 110", rl(1, 9)},   {"0000 0000 0011 101", rl(1, 10)},
    {"0000 0000 0011 100", rl(1, 11)},  {"0000 0000 0011 011", rl(1, 12)},
    {"0000 0000 0011 010", rl(1, 13)},  {"0000 0000 0011 001", rl(1, 14)},
    {"0000 0000 0001 0011", rl(1, 15)}, {"0000 0000 0001 0010", rl(1, 16)},
    {"0000 0000 0001 0001", rl(1, 17)}, {"0000 0000 0001 0000", rl(1, 18)},
    {"0000 0000 0001 0100", rl(6, 3)},  {"0000 0000 0001 1010", rl(11, 2)},
    {"0000 0000 0001 1001", rl(12, 2)}, {"0000 0000 0001 1000", rl(13, 2)},
    {"0000 0000 0001 0111", rl(14, 2)}, {"0000 0000 0001 0110", rl(15, 2)},
    {"0000 0000 0001 0101", rl(16, 2)}, {"0000 0000 0001 1111", rl(27, 1)},
    {"0000 0000 0001 1110", rl(28, 1)}, {"0000 0000 0001 1101", rl(29, 1)},
    {"0000 0000 0001 1100", rl(30, 1)}, {"0000 0000 0001 1011", rl(31, 1)},
};

// Table B-14: DCT coefficients, table zero, without the long codes.
constexpr VlcCode kDctZero[] = {
    {"10", kDctEndOfBlock},
    {"11", rl(0, 1)},
    {"011", rl(1, 1)},
    {"0100", rl(0, 2)},
    {"0101", rl(2, 1)},
    {"0010 1", rl(0, 3)},
    {"0011 1", rl(3, 1)},
    {"0011 0", rl(4, 1)},
    {"0001 10", rl(1, 2)},
    {"0001 11", rl(5, 1)},
    {"0001 01", rl(6, 1)},
    {"0001 00", rl(7, 1)},
    {"0000 110", rl(0, 4)},
    {"0000 100", rl(2, 2)},
    {"0000 111", rl(8, 1)},
    {"0000 101", rl(9, 1)},
    {"0000 01", kDctEscape},
    {"0010 0110", rl(0, 5)},
    {"0010 0001", rl(0, 6)},
    {"0010 0101", rl(1, 3)},
    {"0010 0100", rl(3, 2)},
    {"0010 0111", rl(10, 1)},
    {"0010 0011", rl(11, 1)},
    {"0010 0010", rl(12, 1)},
    {"0010 0000", rl(13, 1)},
    {"0000 0010 10", rl(0, 7)},
    {"0000 0011 00", rl(1, 4)},
    {"0000 0010 11", rl(2, 3)},
    {"0000 0011 11", rl(4, 2)},
    {"0000 0010 01", rl(5, 2)},
    {"0000 0011 10", rl(14, 1)},
    {"0000 0011 01", rl(15, 1)},
    {"0000 0010 00", rl(16, 1)},
    {"0000 0001 1101", rl(0, 8)},
    {"0000 0001 1000", rl(0, 9)},
    {"0000 0001 0011", rl(0, 10)},
    {"0000 0001 0000", rl(0, 11)},
    {"0000 0001 1011", rl(1, 5)},
    {"0000 0001 0100", rl(2, 4)},
    {"0000 0001 1100", rl(3, 3)},
    {"0000 0001 0010", rl(4, 3)},
    {"0000 0001 1110", rl(6, 2)},
    {"0000 0001 0101", rl(7, 2)},
    {"0000 0001 0001", rl(8, 2)},
    {"0000 0001 1111", rl(17, 1)},
    {"0000 0001 1010", rl(18, 1)},
    {"0000 0001 1001", rl(19, 1)},
    {"0000 0001 0111", rl(20, 1)},
    {"0000 0001 0110", rl(21, 1)},
    {"0000 0000 1101 0", rl(0, 12)},
    {"0000 0000 1100 1", rl(0, 13)},
    {"0000 0000 1100 0", rl(0, 14)},
    {"0000 0000 1011 1", rl(0, 15)},
    {"0000 0000 1011 0", rl(1, 6)},
    {"0000 0000 1010 1", rl(1, 7)},
    {"0000 0000 1010 0", rl(2, 5)},
    {"0000 0000 1001 1", rl(3, 4)},
    {"0000 0000 1001 0", rl(5, 3)},
    {"0000 0000 1000 1", rl(9, 2)},
    {"0000 0000 1000 0", rl(10, 2)},
    {"0000 0000 1111 1", rl(22, 1)},
    {"0000 0000 1111 0", rl(23, 1)},
    {"0000 0000 1110 1", rl(24, 1)},
    {"0000 0000 1110 0", rl(25, 1)},
    {"0000 0000 1101 1", rl(26, 1)},
};

// Table B-15: DCT coefficients, table one, without the long codes.
constexpr VlcCode kDctOne[] = {
    {"0110", kDctEndOfBlock},
    {"10", rl(0, 1)},
    {"010", rl(1, 1)},
    {"110", rl(0, 2)},
    {"0010 1", rl(2, 1)},
    {"0111", rl(0, 3)},
    {"0011 1", rl(3, 1)},
    {"0001 10", rl(4, 1)},
    {"0011 0", rl(1, 2)},
    {"0001 11", rl(5, 1)},
    {"0000 110", rl(6, 1)},
    {"0000 100", rl(7, 1)},
    {"1110 0", rl(0, 4)},
    {"0000 111", rl(2, 2)},
    {"0000 101", rl(8, 1)},
    {"1111 000", rl(9, 1)},
    {"0000 01", kDctEscape},
    {"1110 1", rl(0, 5)},
    {"0001 01", rl(0, 6)},
    {"1111 001", rl(1, 3)},
    {"0010 0110", rl(3, 2)},
    {"1111 010", rl(10, 1)},
    {"0010 0001", rl(11, 1)},
    {"0010 0101", rl(12, 1)},
    {"0010 0100", rl(13, 1)},
    {"0001 00", rl(0, 7)},
    {"0010 0111", rl(1, 4)},
    {"1111 1100", rl(2, 3)},
    {"1111 1101", rl(4, 2)},
    {"0000 0010 0", rl(5, 2)},
    {"0000 0010 1", rl(14, 1)},
    {"0000 0011 1", rl(15, 1)},
    {"0000 0011 01", rl(16, 1)},
    {"1111 011", rl(0, 8)},
    {"1111 100", rl(0, 9)},
    {"0010 0011", rl(0, 10)},
    {"0010 0010", rl(0, 11)},
    {"0010 0000", rl(1, 5)},
    {"0000 0011 00", rl(2, 4)},
    {"0000 0001 1100", rl(3, 3)},
    {"0000 0001 0010", rl(4, 3)},
    {"0000 0001 1110", rl(6, 2)},
    {"0000 0001 0101", rl(7, 2)},
    {"0000 0001 0001", rl(8, 2)},
    {"0000 0001 1111", rl(17, 1)},
    {"0000 0001 1010", rl(18, 1)},
    {"0000 0001 1001", rl(19, 1)},
    {"0000 0001 0111", rl(20, 1)},
    {"0000 0001 0110", rl(21, 1)},
    {"1111 1010", rl(0, 12)},
    {"1111 1011", rl(0, 13)},
    {"1111 1110", rl(0, 14)},
    {"1111 1111", rl(0, 15)},
    {"0000 0000 1011 0", rl(1, 6)},
    {"0000 0000 1010 1", rl(1, 7)},
    {"0000 0000 1010 0", rl(2, 5)},
    {"0000 0000 1001 1", rl(3, 4)},
    {"0000 0000 1001 0", rl(5, 3)},
    {"0000 0000 1000 1", rl(9, 2)},
    {"0000 0000 1000 0", rl(10, 2)},
    {"0000 0000 1111 1", rl(22, 1)},
    {"0000 0000 1111 0", rl(23, 1)},
    {"0000 0000 1110 1", rl(24, 1)},
    {"0000 0000 1110 0", rl(25, 1)},
    {"0000 0000 1101 1", rl(26, 1)},
};

}  // namespace

// ---- Building a table ----

VlcTable::VlcTable(std::span<const VlcCode> codes) {
    struct Parsed {
        std::uint32_t code;
        unsigned length;
        std::int16_t value;
    };

    std::vector<Parsed> parsed;

    // Turn each code from its printed text into its bits and length.
    for (const VlcCode& c : codes) {
        Parsed p{0, 0, c.value};

        // The text is read left to right, so the first digit is the most significant bit.
        for (const char ch : c.bits) {
            // The standard prints the bits in groups; the spaces mean nothing.
            if (ch == ' ') {
                continue;
            }

            // Any other character is a typo in the table.
            if (ch != '0' && ch != '1') {
                throw std::logic_error(std::format("bad VLC code \"{}\"", c.bits));
            }

            p.code = p.code << 1 | static_cast<std::uint32_t>(ch == '1');
            ++p.length;
        }

        // A code has at least one bit and fits the table's index.
        if (p.length == 0 || p.length > kMaxCodeBits) {
            throw std::logic_error(std::format("bad VLC code \"{}\"", c.bits));
        }

        m_bits = std::max(m_bits, p.length);
        parsed.push_back(p);
    }

    m_table.assign(std::size_t{1} << m_bits, Entry{});

    // Fill every entry whose leading bits are this code.
    for (const Parsed& p : parsed) {
        const unsigned shift = m_bits - p.length;
        const std::size_t base = static_cast<std::size_t>(p.code) << shift;

        // The bits after the code can be anything.
        for (std::size_t k = 0; k < (std::size_t{1} << shift); ++k) {
            Entry& e = m_table[base + k];

            // Another code already owns this entry, so one code is a prefix of the other.
            if (e.length != 0) {
                throw std::logic_error("overlapping codes in a VLC table");
            }

            e = Entry{p.value, static_cast<std::uint8_t>(p.length)};
        }
    }
}

double VlcTable::kraft() const {
    std::size_t used = 0;

    // Count the entries that some code owns.
    for (const Entry& e : m_table) {
        used += e.length != 0;
    }

    return static_cast<double>(used) / static_cast<double>(m_table.size());
}

// ---- The standard's tables ----

std::span<const VlcCode> mpeg2_codes(Mpeg2Codes which) {
    // One list for each enumerator; there is no default, so the compiler warns when one is added.
    switch (which) {
        case Mpeg2Codes::MacroblockAddress:
            return kMacroblockAddress;
        case Mpeg2Codes::MacroblockTypeI:
            return kMacroblockTypeI;
        case Mpeg2Codes::MacroblockTypeP:
            return kMacroblockTypeP;
        case Mpeg2Codes::MacroblockTypeB:
            return kMacroblockTypeB;
        case Mpeg2Codes::CodedBlockPattern:
            return kCodedBlockPattern;
        case Mpeg2Codes::MotionCode:
            return kMotionCode;
        case Mpeg2Codes::DcSizeLuminance:
            return kDcSizeLuminance;
        case Mpeg2Codes::DcSizeChrominance:
            return kDcSizeChrominance;
        case Mpeg2Codes::DctZero:
            return kDctZero;
        case Mpeg2Codes::DctOne:
            return kDctOne;
        case Mpeg2Codes::DctLong:
            return kDctLong;
    }

    // Not reached for an enumerator of Mpeg2Codes.
    return {};
}

namespace {

// Returns the codes of `table` followed by the long codes B-14 and B-15 share.
std::vector<VlcCode> with_long_codes(std::span<const VlcCode> table) {
    std::vector<VlcCode> all(table.begin(), table.end());
    all.insert(all.end(), std::begin(kDctLong), std::end(kDctLong));
    return all;
}

}  // namespace

// Builds the tables on the first call and keeps them. The static is initialised once, even when
// several threads ask at the same time.
const Mpeg2VlcTables& mpeg2_vlc_tables() {
    static const Mpeg2VlcTables tables{
        VlcTable(kMacroblockAddress),
        {VlcTable(kMacroblockTypeI), VlcTable(kMacroblockTypeP), VlcTable(kMacroblockTypeB)},
        VlcTable(kCodedBlockPattern),
        VlcTable(kMotionCode),
        VlcTable(kDcSizeLuminance),
        VlcTable(kDcSizeChrominance),
        VlcTable(with_long_codes(kDctZero)),
        VlcTable(with_long_codes(kDctOne)),
    };
    return tables;
}

}  // namespace openrac::media
