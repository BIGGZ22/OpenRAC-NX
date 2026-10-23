// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tfrag_light.rs
// (module ps2) and crates/rc-engine/src/tie_lod.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The console's float arithmetic for the world renderers: the shared model
// (common/ps2_float.h) under the names the renderers use, and the tie
// shader's integer-only versions of it, kept beside the model so the tests
// check the GLSL's arithmetic against it.

#pragma once

#include <cstdint>

#include "common/ps2_float.h"

namespace openrac::renderer::world::vu {

inline constexpr std::uint32_t kSign = ps2::kSign;
inline constexpr std::uint32_t kMax = ps2::kMax;
inline constexpr std::uint32_t kOne = ps2::kOne;

inline std::uint32_t mul(std::uint32_t a, std::uint32_t b) {
    return ps2::mul(a, b);
}

inline std::uint32_t add(std::uint32_t a, std::uint32_t b) {
    return ps2::add(a, b);
}

inline std::uint32_t sub(std::uint32_t a, std::uint32_t b) {
    return ps2::sub(a, b);
}

// Division by zero gives +-kMax.
inline std::uint32_t div(std::uint32_t a, std::uint32_t b) {
    return ps2::div(a, b);
}

inline std::uint32_t max(std::uint32_t a, std::uint32_t b) {
    return ps2::max(a, b);
}

inline std::uint32_t min(std::uint32_t a, std::uint32_t b) {
    return ps2::min(a, b);
}

inline std::uint32_t bits(float f) {
    return ps2::bits(f);
}

inline float value(std::uint32_t b) {
    return ps2::to_host(b);
}

// VU ftoi0: truncation toward zero, saturating.
inline std::int32_t ftoi0(float x) {
    return ps2::ftoi(ps2::bits(x), 0);
}

// The tie shader's integer-only versions of mul() and add() for non-negative
// operands (tie.vert's vu_mul / vu_add, line for line): kept here so the
// tests check the GLSL's arithmetic against the float model.
std::uint32_t mul_positive(std::uint32_t a, std::uint32_t b);
std::uint32_t add_positive(std::uint32_t a, std::uint32_t b);

// One colour lane of a tie "fat" vertex as the tie VU1 program computes it
// (ReRAC's reading of program 13507): the palette lanes are 0x4b000000 + byte
// (an unpack with a 2^23 row), the second and third slots are averaged (x 0.5),
// then weighted avg * w + c0 * z, every step truncated, and the GS keeps the
// low byte. `w` and `z` are the instance's colour weights (TieLodPick).
std::uint8_t fat_colour_lane(
    std::uint8_t c0, std::uint8_t c1, std::uint8_t c2, std::uint32_t w, std::uint32_t z
);

}  // namespace openrac::renderer::world::vu
