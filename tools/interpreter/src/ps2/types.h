// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The integer aliases and the bit helpers every other file of the PlayStation 2 model uses.
 *
 * The aliases give a hardware value (a register, an address, a field, a pixel) the exact width it
 * has on the console. `bits` cuts a field out of a register word. `load` and `store` move a value
 * between a byte pointer and a typed value, and `as_float` and `as_u32` reinterpret a 32-bit word
 * as a float and back. Nothing in it knows about a unit of the console.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace ps2 {

/** Fixed-width integers: `u` is unsigned, `s` is signed, the number is the width in bits. */
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

/**
 * Cuts a field of `count` bits starting at bit `start` out of a word.
 *
 * Used to decode register and packet layouts, which the hardware documentation gives as bit ranges.
 *
 * @param value The word to cut the field from.
 * @param start Number of the lowest bit of the field, counted from bit 0 as the least significant.
 * @param count Width of the field in bits, 63 at most.
 * @return The field, shifted down to bit 0.
 */
constexpr u64 bits(u64 value, unsigned start, unsigned count) {
    return (value >> start) & ((u64{1} << count) - 1);
}

/**
 * Reads a `T` from `p` without assuming that `p` is aligned for it.
 *
 * The PlayStation 2 is little-endian and so is every host this is built for; memcpy keeps
 * unaligned reads defined.
 *
 * @tparam T A trivially copyable type, usually an integer or a float.
 * @param p Address of the first byte of the value.
 * @return The value, with the bytes read in host order.
 */
template <typename T>
inline T load(const void* p) {
    T v;
    std::memcpy(&v, p, sizeof(T));
    return v;
}

/**
 * Writes `v` to `p` without assuming that `p` is aligned for a `T`.
 *
 * @tparam T A trivially copyable type, usually an integer or a float.
 * @param[out] p Address of the first byte to write.
 * @param v The value to write, in host byte order.
 */
template <typename T>
inline void store(void* p, T v) {
    std::memcpy(p, &v, sizeof(T));
}

/**
 * Reinterprets the bits of a 32-bit word as a float.
 *
 * @param v The word.
 * @return The float with the same bit pattern.
 */
inline float as_float(u32 v) {
    return load<float>(&v);
}

/**
 * Reinterprets the bits of a float as a 32-bit word.
 *
 * @param v The float.
 * @return The word with the same bit pattern.
 */
inline u32 as_u32(float v) {
    return load<u32>(&v);
}

}  // namespace ps2
