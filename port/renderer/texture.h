// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Texture conversion: the console's pixel formats to RGBA8, once, on the CPU,
// when a texture is loaded or first used (OpenGOAL's way; RENDERER.md
// section 5). Each function takes pixels as the game hands them over: in
// raster order (row by row, left to right), which is how a host-to-GS image
// transfer sends a rectangle in its own format. The chip's internal layout
// only matters when a texture is uploaded in one format and read in another
// (an 8-bit texture sent as 32-bit pixels, for instance); reinterpret() below
// handles that case and nothing else needs it.
//
// Alpha: the console's alpha runs from 0 to 0x80 (opaque), and up to 0xFF.
// AlphaScale::Gs keeps that (the direct renderer's shaders read 0x80 as 1.0);
// AlphaScale::Full doubles it, clamped, for ordinary GL use.

#pragma once

#include <algorithm>
#include <cstdint>
#include <span>
#include <vector>

#include "renderer/gs.h"

namespace openrac::renderer {

struct Rgba8Image {
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> pixels;  // 4 bytes per texel, top row first

    Rgba8Image() = default;

    // A negative side is an empty image. The size is reckoned in std::size_t: w * h * 4 in int
    // overflows for sizes a garbage TEX0 can claim (up to 32768 x 32768): in 0.8.0 one such as
    // 32768 x 16384 came out as -2^31, asked operator new for 2^64 - 2^31 bytes and threw
    // bad_alloc (Kerwan, crash report 2026-10-19).
    Rgba8Image(int w, int h)
        : width(std::max(w, 0)),
          height(std::max(h, 0)),
          pixels(static_cast<std::size_t>(std::max(w, 0)) * static_cast<std::size_t>(std::max(h, 0)) * 4) {}

    std::uint32_t texel(int x, int y) const;  // R in the low byte
    void set_texel(int x, int y, std::uint32_t rgba);
};

enum class AlphaScale {
    Gs,
    Full
};

// RGBA as a little-endian word: R in bits 0-7, A in bits 24-31 (the order
// of a PSMCT32 pixel, and of GL_RGBA bytes in memory).
constexpr std::uint32_t rgba(std::uint32_t r, std::uint32_t g, std::uint32_t b, std::uint32_t a) {
    return r | (g << 8) | (b << 16) | (a << 24);
}

std::uint32_t scale_alpha(std::uint32_t colour, AlphaScale scale);

// One 16-bit pixel (A1 B5 G5 R5) to RGBA, its alpha from TEXA.
std::uint32_t ct16_to_rgba(std::uint16_t pixel, const gs::Texa& texa);

// ---- Direct colour textures ----
Rgba8Image convert_ct32(
    std::span<const std::uint8_t> data, int width, int height, AlphaScale scale
);
Rgba8Image convert_ct24(
    std::span<const std::uint8_t> data,
    int width,
    int height,
    const gs::Texa& texa,
    AlphaScale scale
);
Rgba8Image convert_ct16(
    std::span<const std::uint8_t> data,
    int width,
    int height,
    const gs::Texa& texa,
    AlphaScale scale
);

// ---- Palettes ----
// The colours of a CLUT in index order. A CLUT is itself an image the game
// uploads: 16 entries as an 8 x 2 image, or 256 as a 16 x 16 one. In CSM1
// (the storage the games use) a 256-entry CLUT is stored with bits 3 and 4 of
// the index swapped: entry i is pixel csm1_position(i) of the image.
constexpr std::uint32_t csm1_position(std::uint32_t index) {
    return (index & ~0x18u) | ((index & 0x08u) << 1) | ((index & 0x10u) >> 1);
}

// `data` is the CLUT image in raster order, in its format (PSMCT32 or
// PSMCT16), `image_width` pixels to a row (8 or 16, or wider when the game
// uploads a larger rectangle); `entries` is 16 or 256.
std::vector<std::uint32_t> clut_csm1(
    std::span<const std::uint8_t> data,
    std::uint8_t cpsm,
    int image_width,
    int entries,
    const gs::Texa& texa
);

// The largest texture side the chip samples: TW and TH go up to 10 (1024 texels). The registers
// have room for 15 (32768), which only a garbage write uses.
constexpr int kMaxTextureSide = 1024;

// Any supported format to RGBA8: `palette` is used by the indexed formats
// (PSMT8, PSMT4 and their "H" forms, whose raster data is plain indices). A side
// outside 1..kMaxTextureSide is no texture the chip has: the result is empty
// (0 x 0), never an allocation of gigabytes.
Rgba8Image convert(
    std::span<const std::uint8_t> data,
    std::uint8_t psm,
    int width,
    int height,
    std::span<const std::uint32_t> palette,
    const gs::Texa& texa,
    AlphaScale scale
);

// The palette size of an indexed format (256 or 16), 0 for the others.
int palette_entries(std::uint8_t psm);

// ---- Indexed textures ----
Rgba8Image convert_t8(
    std::span<const std::uint8_t> indices,
    int width,
    int height,
    std::span<const std::uint32_t> palette,
    AlphaScale scale
);
// Two texels per byte, the left one in the low nibble.
Rgba8Image convert_t4(
    std::span<const std::uint8_t> indices,
    int width,
    int height,
    std::span<const std::uint32_t> palette,
    AlphaScale scale
);

// ---- The chip's layout, for textures uploaded in a format they are not read in ----
//
// The GS stores pixels in 8 KB pages of 32 blocks, each block in four
// columns, with an arrangement per format (the chip's public documentation).
// These give a pixel's position in that layout, in 4-bit units from the
// start of block `bp`, for a buffer `bw` 64-pixel units wide. They exist only
// for reinterpret(): the renderer keeps no image of the chip's memory.
namespace swizzle {
std::uint64_t nibble_address(std::uint8_t psm, int x, int y, std::uint32_t bp, std::uint32_t bw);
}  // namespace swizzle

// Pixels uploaded as a `from_psm` rectangle (raster order, at block `bp`,
// buffer width `from_bw`) read back as a `to_psm` texture of to_width x
// to_height at the same base with buffer width `to_bw`; returned in raster
// order of `to_psm`. Formats: PSMCT32, PSMCT24, PSMCT16, PSMT8, PSMT4.
std::vector<std::uint8_t> reinterpret(
    std::span<const std::uint8_t> data,
    std::uint8_t from_psm,
    int from_width,
    int from_height,
    std::uint32_t from_bw,
    std::uint8_t to_psm,
    int to_width,
    int to_height,
    std::uint32_t to_bw
);

// The same, for a rectangle placed at (`from_x`, `from_y`) of its buffer, read as a texture whose
// base is `to_block` blocks past the rectangle's base: a texture or CLUT the game uploaded inside a
// larger transfer, or a transfer into the middle of a buffer. Texels the rectangle does not cover
// read as 0.
std::vector<std::uint8_t> reinterpret_at(
    std::span<const std::uint8_t> data,
    std::uint8_t from_psm,
    int from_x,
    int from_y,
    int from_width,
    int from_height,
    std::uint32_t from_bw,
    std::uint32_t to_block,
    std::uint8_t to_psm,
    int to_width,
    int to_height,
    std::uint32_t to_bw
);

// Bits of one pixel in a raster transfer: 24 for PSMCT24, 8 and 4 for the
// "H" formats (indices only), otherwise the format's own size.
int raster_bits(std::uint8_t psm);

// Bytes of a width x height rectangle in a format (rounded up for 4-bit).
std::size_t image_bytes(std::uint8_t psm, int width, int height);

}  // namespace openrac::renderer
