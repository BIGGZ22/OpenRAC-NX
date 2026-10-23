// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <bit>
#include <cstdint>

namespace openrac::float_encoding {
// IEEE float to a five-bit-exponent float, rounded to nearest with ties to even.
// Used for half-float metadata and the unsigned 11/11/10-bit irradiance channels.
inline std::uint32_t pack_small_float(float value, unsigned mantissa_bits, bool signed_value) {
    const auto bits = std::bit_cast<std::uint32_t>(value);
    const auto sign = signed_value ? (bits >> 31) << (mantissa_bits + 5) : 0u;
    if (!signed_value && (bits >> 31)) {
        return 0;
    }
    const unsigned exponent = (bits >> 23) & 255;
    const auto mantissa = bits & 0x7FFFFF;
    if (exponent == 255) {
        return sign | (31u << mantissa_bits) | (mantissa ? 1u << (mantissa_bits - 1) : 0u);
    }
    const int reduced_exponent = static_cast<int>(exponent) - 127 + 15;
    if (reduced_exponent >= 31) {
        return sign | (31u << mantissa_bits);
    }
    const unsigned shift = reduced_exponent <= 0
                               ? static_cast<unsigned>(24 - mantissa_bits - reduced_exponent)
                               : 23 - mantissa_bits;
    if (shift > 24) {
        return sign;
    }
    const auto significand = reduced_exponent <= 0 ? mantissa | 0x800000u : mantissa;
    auto rounded = significand >> shift;
    const auto remainder = significand & ((1u << shift) - 1);
    const auto halfway = 1u << (shift - 1);
    if (remainder > halfway || (remainder == halfway && (rounded & 1))) {
        ++rounded;
    }
    // Addition carries a rounded mantissa into the exponent.
    return sign
           | ((reduced_exponent > 0 ? static_cast<unsigned>(reduced_exponent) << mantissa_bits : 0u)
              + rounded);
}

}  // namespace openrac::float_encoding
