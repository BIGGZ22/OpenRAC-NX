// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/texture.h"

#include <algorithm>
#include <cstring>

namespace openrac::renderer {

std::uint32_t Rgba8Image::texel(int x, int y) const {
    std::uint32_t v = 0;
    std::memcpy(&v, &pixels[static_cast<std::size_t>((y * width + x) * 4)], 4);
    return v;
}

void Rgba8Image::set_texel(int x, int y, std::uint32_t v) {
    std::memcpy(&pixels[static_cast<std::size_t>((y * width + x) * 4)], &v, 4);
}

std::uint32_t scale_alpha(std::uint32_t colour, AlphaScale scale) {
    if (scale == AlphaScale::Gs) {
        return colour;
    }
    const std::uint32_t a = std::min<std::uint32_t>(255, (colour >> 24) * 2);
    return (colour & 0x00FFFFFFu) | (a << 24);
}

std::uint32_t ct16_to_rgba(std::uint16_t pixel, const gs::Texa& texa) {
    // The chip widens each 5-bit channel by shifting it left 3, low bits zero.
    const std::uint32_t r = (pixel & 0x1Fu) << 3;
    const std::uint32_t g = ((pixel >> 5) & 0x1Fu) << 3;
    const std::uint32_t b = ((pixel >> 10) & 0x1Fu) << 3;
    std::uint32_t a = 0;
    if ((pixel & 0x8000u) != 0) {
        a = texa.ta1;
    } else if (!(texa.aem && r == 0 && g == 0 && b == 0)) {
        a = texa.ta0;
    }
    return rgba(r, g, b, a);
}

namespace {

std::uint32_t read32(std::span<const std::uint8_t> data, std::size_t offset) {
    std::uint32_t v = 0;
    if (offset + 4 <= data.size()) {
        std::memcpy(&v, data.data() + offset, 4);
    }
    return v;
}

std::uint16_t read16(std::span<const std::uint8_t> data, std::size_t offset) {
    if (offset + 2 > data.size()) {
        return 0;
    }
    return static_cast<std::uint16_t>(data[offset] | (data[offset + 1] << 8));
}

std::uint8_t read8(std::span<const std::uint8_t> data, std::size_t offset) {
    return offset < data.size() ? data[offset] : 0;
}

std::uint8_t read4(std::span<const std::uint8_t> data, std::size_t index) {
    const std::uint8_t byte = read8(data, index / 2);
    return static_cast<std::uint8_t>((index & 1) != 0 ? byte >> 4 : byte & 0x0F);
}

}  // namespace

Rgba8Image convert_ct32(
    std::span<const std::uint8_t> data, int width, int height, AlphaScale scale
) {
    Rgba8Image out(width, height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const auto i = static_cast<std::size_t>(y * width + x);
            out.set_texel(x, y, scale_alpha(read32(data, i * 4), scale));
        }
    }
    return out;
}

Rgba8Image convert_ct24(
    std::span<const std::uint8_t> data,
    int width,
    int height,
    const gs::Texa& texa,
    AlphaScale scale
) {
    Rgba8Image out(width, height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const auto i = static_cast<std::size_t>(y * width + x) * 3;
            const std::uint32_t r = read8(data, i);
            const std::uint32_t g = read8(data, i + 1);
            const std::uint32_t b = read8(data, i + 2);
            const std::uint32_t a = (texa.aem && r == 0 && g == 0 && b == 0) ? 0 : texa.ta0;
            out.set_texel(x, y, scale_alpha(rgba(r, g, b, a), scale));
        }
    }
    return out;
}

Rgba8Image convert_ct16(
    std::span<const std::uint8_t> data,
    int width,
    int height,
    const gs::Texa& texa,
    AlphaScale scale
) {
    Rgba8Image out(width, height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const auto i = static_cast<std::size_t>(y * width + x) * 2;
            out.set_texel(x, y, scale_alpha(ct16_to_rgba(read16(data, i), texa), scale));
        }
    }
    return out;
}

std::vector<std::uint32_t> clut_csm1(
    std::span<const std::uint8_t> data,
    std::uint8_t cpsm,
    int image_width,
    int entries,
    const gs::Texa& texa
) {
    std::vector<std::uint32_t> out(static_cast<std::size_t>(entries));
    const bool is16 = cpsm == gs::kPsmct16 || cpsm == gs::kPsmct16s;
    // 16 entries are an 8 x 2 image in index order; 256 are a 16 x 16 one
    // with bits 3 and 4 of the index swapped.
    const std::uint32_t row = entries == 256 ? 16 : 8;
    for (int i = 0; i < entries; ++i) {
        const std::uint32_t position = entries == 256 ? csm1_position(static_cast<std::uint32_t>(i))
                                                      : static_cast<std::uint32_t>(i);
        const std::size_t pixel =
            (position / row) * static_cast<std::size_t>(image_width) + position % row;
        out[static_cast<std::size_t>(i)] =
            is16 ? ct16_to_rgba(read16(data, pixel * 2), texa) : read32(data, pixel * 4);
    }
    return out;
}

Rgba8Image convert_t8(
    std::span<const std::uint8_t> indices,
    int width,
    int height,
    std::span<const std::uint32_t> palette,
    AlphaScale scale
) {
    Rgba8Image out(width, height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const std::uint8_t index = read8(indices, static_cast<std::size_t>(y * width + x));
            const std::uint32_t colour = index < palette.size() ? palette[index] : 0;
            out.set_texel(x, y, scale_alpha(colour, scale));
        }
    }
    return out;
}

Rgba8Image convert_t4(
    std::span<const std::uint8_t> indices,
    int width,
    int height,
    std::span<const std::uint32_t> palette,
    AlphaScale scale
) {
    Rgba8Image out(width, height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const std::uint8_t index = read4(indices, static_cast<std::size_t>(y * width + x));
            const std::uint32_t colour = index < palette.size() ? palette[index] : 0;
            out.set_texel(x, y, scale_alpha(colour, scale));
        }
    }
    return out;
}

int palette_entries(std::uint8_t psm) {
    switch (psm) {
        case gs::kPsmt8:
        case gs::kPsmt8h:
            return 256;
        case gs::kPsmt4:
        case gs::kPsmt4hl:
        case gs::kPsmt4hh:
            return 16;
        default:
            return 0;
    }
}

Rgba8Image convert(
    std::span<const std::uint8_t> data,
    std::uint8_t psm,
    int width,
    int height,
    std::span<const std::uint32_t> palette,
    const gs::Texa& texa,
    AlphaScale scale
) {
    switch (psm) {
        case gs::kPsmct32:
            return convert_ct32(data, width, height, scale);
        case gs::kPsmct24:
            return convert_ct24(data, width, height, texa, scale);
        case gs::kPsmct16:
        case gs::kPsmct16s:
            return convert_ct16(data, width, height, texa, scale);
        case gs::kPsmt8:
        case gs::kPsmt8h:
            return convert_t8(data, width, height, palette, scale);
        case gs::kPsmt4:
        case gs::kPsmt4hl:
        case gs::kPsmt4hh:
            return convert_t4(data, width, height, palette, scale);
        default:
            return Rgba8Image(width, height);
    }
}

int raster_bits(std::uint8_t psm) {
    switch (psm) {
        case gs::kPsmct24:
        case gs::kPsmz24:
            return 24;  // a 24-bit transfer packs three bytes a pixel
        case gs::kPsmt8h:
            return 8;
        case gs::kPsmt4hl:
        case gs::kPsmt4hh:
            return 4;
        default:
            return gs::bits_per_pixel(psm);
    }
}

std::size_t image_bytes(std::uint8_t psm, int width, int height) {
    const auto pixels = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    return (pixels * static_cast<std::size_t>(raster_bits(psm)) + 7) / 8;
}

namespace swizzle {
namespace {

// Where each block sits in a page, by block row and column. The 32- and
// 8-bit formats have pages 8 blocks wide and 4 high; the 16- and 4-bit
// formats 4 wide and 8 high.
constexpr std::uint8_t kBlocks8x4[4][8] = {
    {0, 1, 4, 5, 16, 17, 20, 21},
    {2, 3, 6, 7, 18, 19, 22, 23},
    {8, 9, 12, 13, 24, 25, 28, 29},
    {10, 11, 14, 15, 26, 27, 30, 31},
};
constexpr std::uint8_t kBlocks4x8[8][4] = {
    {0, 2, 8, 10},
    {1, 3, 9, 11},
    {4, 6, 12, 14},
    {5, 7, 13, 15},
    {16, 18, 24, 26},
    {17, 19, 25, 27},
    {20, 22, 28, 30},
    {21, 23, 29, 31},
};

// A column is 64 bytes: 16 words, laid out as two rows of eight. Every
// format fills a column's words in this order, two pixel rows at a time.
constexpr std::uint8_t kColumnWords[2][8] = {
    {0, 1, 4, 5, 8, 9, 12, 13},
    {2, 3, 6, 7, 10, 11, 14, 15},
};

// Within a block, in 4-bit units.
std::uint32_t in_block(std::uint8_t psm, std::uint32_t x, std::uint32_t y) {
    switch (psm) {
        case gs::kPsmct32:
        case gs::kPsmct24:
        case gs::kPsmz32:
        case gs::kPsmz24: {
            // 8 x 8 pixels; a column is 8 x 2.
            const std::uint32_t column = (y >> 1) & 3;
            const std::uint32_t word = column * 16 + kColumnWords[y & 1][x & 7];
            return word * 8;
        }
        case gs::kPsmct16:
        case gs::kPsmct16s:
        case gs::kPsmz16:
        case gs::kPsmz16s: {
            // 16 x 8 pixels; a column is 16 x 2, the right half in the high
            // halfwords.
            const std::uint32_t column = (y >> 1) & 3;
            const std::uint32_t half =
                column * 32 + kColumnWords[y & 1][x & 7] * 2 + ((x >> 3) & 1);
            return half * 4;
        }
        case gs::kPsmt8: {
            // 16 x 16 pixels; a column is 16 x 4. Rows 0-1 of a column fill
            // bytes 0 and 2 of its words, rows 2-3 bytes 1 and 3; every other
            // pair of rows is shifted four words along (alternating by column).
            const std::uint32_t column = (y >> 2) & 3;
            const std::uint32_t row = y & 3;
            const bool shift = ((row >> 1) ^ (column & 1)) != 0;
            const std::uint32_t xx = shift ? (x + 4) & 7 : x & 7;
            const std::uint32_t word = column * 16 + kColumnWords[row & 1][xx];
            const std::uint32_t byte = (row >> 1) + ((x >> 3) & 1) * 2;
            return (word * 4 + byte) * 2;
        }
        case gs::kPsmt4: {
            // 32 x 16 pixels; a column is 32 x 4, the same pattern as 8-bit
            // with eight nibbles to a word.
            const std::uint32_t column = (y >> 2) & 3;
            const std::uint32_t row = y & 3;
            const bool shift = ((row >> 1) ^ (column & 1)) != 0;
            const std::uint32_t xx = shift ? (x + 4) & 7 : x & 7;
            const std::uint32_t word = column * 16 + kColumnWords[row & 1][xx];
            const std::uint32_t nibble = (row >> 1) + ((x >> 3) & 3) * 2;
            return word * 8 + nibble;
        }
        default:
            return 0;
    }
}

}  // namespace

std::uint64_t nibble_address(std::uint8_t psm, int x, int y, std::uint32_t bp, std::uint32_t bw) {
    const auto ux = static_cast<std::uint32_t>(x);
    const auto uy = static_cast<std::uint32_t>(y);
    std::uint64_t block = bp;
    switch (psm) {
        case gs::kPsmct32:
        case gs::kPsmct24:
        case gs::kPsmz32:
        case gs::kPsmz24:
            // Pages of 64 x 32.
            block += (static_cast<std::uint64_t>(uy / 32) * bw + ux / 64) * 32
                     + kBlocks8x4[(uy / 8) % 4][(ux / 8) % 8];
            break;
        case gs::kPsmct16:
        case gs::kPsmct16s:
        case gs::kPsmz16:
        case gs::kPsmz16s:
            // Pages of 64 x 64.
            block += (static_cast<std::uint64_t>(uy / 64) * bw + ux / 64) * 32
                     + kBlocks4x8[(uy / 8) % 8][(ux / 16) % 4];
            break;
        case gs::kPsmt8: {
            // Pages of 128 x 64; the buffer width counts 64-pixel units.
            const std::uint64_t pages = std::max<std::uint32_t>(bw / 2, 1);
            block += (static_cast<std::uint64_t>(uy / 64) * pages + ux / 128) * 32
                     + kBlocks8x4[(uy / 16) % 4][(ux / 16) % 8];
            break;
        }
        case gs::kPsmt4: {
            // Pages of 128 x 128.
            const std::uint64_t pages = std::max<std::uint32_t>(bw / 2, 1);
            block += (static_cast<std::uint64_t>(uy / 128) * pages + ux / 128) * 32
                     + kBlocks4x8[(uy / 16) % 8][(ux / 32) % 4];
            break;
        }
        default:
            break;
    }
    return block * 512 + in_block(psm, ux, uy);
}

}  // namespace swizzle

namespace {

// The layout a format is read through: the "H" formats are indices kept in
// the top bits of 32-bit pixels.
std::uint8_t layout_of(std::uint8_t psm) {
    switch (psm) {
        case gs::kPsmt8h:
        case gs::kPsmt4hl:
        case gs::kPsmt4hh:
            return gs::kPsmct32;
        default:
            return psm;
    }
}

// The nibble of a 32-bit pixel where an "H" format's index starts.
std::uint32_t high_offset(std::uint8_t psm) {
    switch (psm) {
        case gs::kPsmt8h:
            return 6;
        case gs::kPsmt4hl:
            return 6;
        case gs::kPsmt4hh:
            return 7;
        default:
            return 0;
    }
}

// Nibbles of one pixel as it is stored (24-bit pixels occupy a 32-bit slot, of which only the low
// six nibbles are the pixel's: as many as its raster carries, so a rectangle read as PSMZ24 fills
// exactly its raster and no nibble of the next pixel lands in the slot's top byte).
int stored_nibbles(std::uint8_t psm) {
    switch (psm) {
        case gs::kPsmct24:
        case gs::kPsmz24:
            return 6;
        case gs::kPsmt8h:
            return 2;
        case gs::kPsmt4hl:
        case gs::kPsmt4hh:
            return 1;
        default:
            return gs::bits_per_pixel(psm) / 4;
    }
}

// Nibbles of one pixel in a raster transfer.
int raster_nibbles(std::uint8_t psm) {
    return raster_bits(psm) / 4;
}

std::uint8_t get_nibble(std::span<const std::uint8_t> data, std::uint64_t index) {
    const std::uint64_t byte = index / 2;
    if (byte >= data.size()) {
        return 0;
    }
    return static_cast<std::uint8_t>((index & 1) != 0 ? data[byte] >> 4 : data[byte] & 0x0F);
}

void put_nibble(std::vector<std::uint8_t>& data, std::uint64_t index, std::uint8_t value) {
    std::uint8_t& byte = data[static_cast<std::size_t>(index / 2)];
    if ((index & 1) != 0) {
        byte = static_cast<std::uint8_t>((byte & 0x0F) | (value << 4));
    } else {
        byte = static_cast<std::uint8_t>((byte & 0xF0) | (value & 0x0F));
    }
}

}  // namespace

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
) {
    return reinterpret_at(data, from_psm, 0, 0, from_width, from_height, from_bw, 0, to_psm, to_width, to_height,
                          to_bw);
}

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
) {
    const std::uint8_t from_layout = layout_of(from_psm);
    const std::uint8_t to_layout = layout_of(to_psm);
    const int from_nibbles = stored_nibbles(from_psm);
    const int to_nibbles = stored_nibbles(to_psm);
    const std::uint32_t from_high = high_offset(from_psm);
    const std::uint32_t to_high = high_offset(to_psm);

    // Only the span both rectangles touch is held, for this one texture.
    std::uint64_t end = 0;
    for (int y = 0; y < from_height; ++y) {
        for (int x = 0; x < from_width; ++x) {
            end = std::max(end, swizzle::nibble_address(from_layout, x + from_x, y + from_y, 0, from_bw) + 8);
        }
    }
    for (int y = 0; y < to_height; ++y) {
        for (int x = 0; x < to_width; ++x) {
            end = std::max(end, swizzle::nibble_address(to_layout, x, y, to_block, to_bw) + 8);
        }
    }
    std::vector<std::uint8_t> scratch(static_cast<std::size_t>((end + 1) / 2));

    const int from_raster = raster_nibbles(from_psm);
    for (int y = 0; y < from_height; ++y) {
        for (int x = 0; x < from_width; ++x) {
            const std::uint64_t at =
                swizzle::nibble_address(from_layout, x + from_x, y + from_y, 0, from_bw) + from_high;
            const auto source = static_cast<std::uint64_t>(y * from_width + x)
                                * static_cast<std::uint64_t>(from_raster);
            for (int n = 0; n < from_nibbles; ++n) {
                put_nibble(
                    scratch,
                    at + static_cast<std::uint64_t>(n),
                    get_nibble(data, source + static_cast<std::uint64_t>(n))
                );
            }
        }
    }

    const int to_raster = raster_nibbles(to_psm);
    std::vector<std::uint8_t> out(
        (static_cast<std::size_t>(to_width) * static_cast<std::size_t>(to_height)
             * static_cast<std::size_t>(to_raster)
         + 1)
        / 2
    );
    for (int y = 0; y < to_height; ++y) {
        for (int x = 0; x < to_width; ++x) {
            const std::uint64_t at = swizzle::nibble_address(to_layout, x, y, to_block, to_bw) + to_high;
            const auto dest = static_cast<std::uint64_t>(y * to_width + x)
                              * static_cast<std::uint64_t>(to_raster);
            for (int n = 0; n < to_nibbles; ++n) {
                put_nibble(
                    out,
                    dest + static_cast<std::uint64_t>(n),
                    get_nibble(scratch, at + static_cast<std::uint64_t>(n))
                );
            }
        }
    }
    return out;
}

}  // namespace openrac::renderer
