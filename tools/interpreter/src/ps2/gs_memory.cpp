// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The pixel layouts of GS local memory, declared in gs_memory.h.
 *
 * The layouts are written twice on purpose. The address functions work out a pixel's place from
 * its coordinates with small tables for the columns. The `Layout` tables split the same sum into
 * the parts that depend on x and on y, so an inner loop adds two table reads. The tests compare
 * the two.
 *
 * Sources: the GS's local memory organisation (pages, blocks, columns) as publicly documented.
 */

#include "gs_memory.h"

#include <array>

namespace ps2 {
namespace {

/**
 * Returns the number of a block within a wide page, for the 32-bit and 8-bit formats.
 *
 * In the 32-bit and 8-bit formats a page is 8 blocks wide and 4 high and the block number
 * interleaves the bits of the block's column and row, column first. In the 16-bit and 4-bit
 * formats a page is 4 wide and 8 high and the row comes first. PSMCT16S moves the row's top bit
 * down. The Z formats use the same orders with the two top bits inverted (documented).
 *
 * @param bx Block column within the page, 0-7.
 * @param by Block row within the page, 0-3.
 * @return The block number within the page, 0-31.
 */
constexpr u32 block_wide(u32 bx, u32 by) {
    return (bx & 1) | ((by & 1) << 1) | ((bx & 2) << 1) | ((by & 2) << 2) | ((bx & 4) << 2);
}

/**
 * Returns the number of a block within a tall page, for the 16-bit and 4-bit formats.
 *
 * The row comes first, as the description at `block_wide` says.
 *
 * @param bx Block column within the page, 0-3.
 * @param by Block row within the page, 0-7.
 * @return The block number within the page, 0-31.
 */
constexpr u32 block_tall(u32 bx, u32 by) {
    return (by & 1) | ((bx & 1) << 1) | ((by & 2) << 1) | ((bx & 2) << 2) | ((by & 4) << 2);
}

/**
 * Returns the number of a block within a tall page of PSMCT16S and PSMZ16S.
 *
 * It is `block_tall` with the row's top bit moved down, as the description at `block_wide` says.
 *
 * @param bx Block column within the page, 0-3.
 * @param by Block row within the page, 0-7.
 * @return The block number within the page, 0-31.
 */
constexpr u32 block_tall_s(u32 bx, u32 by) {
    return (by & 1) | ((bx & 1) << 1) | (by & 4) | ((by & 2) << 2) | ((bx & 2) << 3);
}

/**
 * Returns the number of a word within a column of 16 words.
 *
 * A block is four columns of sixteen 32-bit words. A column holds eight words per pixel row pair,
 * ordered by (x bit 0, y bit 0, then the rest of x). Narrower formats pack several pixels into
 * each word: the part of the word a pixel takes depends on which eighth of the column row it is in
 * and on which half of the column's rows, and in the second half (the first half in odd columns)
 * the two groups of four words swap (documented).
 *
 * @param x8 The pixel's x within the column, 0-7.
 * @param y2 The pixel's row within the column's pair of rows, 0 or 1.
 * @return The word within the column, 0-15.
 */
constexpr u32 column_word(u32 x8, u32 y2) {
    return (x8 & 1) | ((y2 & 1) << 1) | ((x8 >> 1) << 2);
}

/**
 * The place of a pixel within a block, for each format width, as tables made at compile time.
 *
 * Each table is indexed by the pixel's row and column within the block. Its entry is the index
 * of the pixel's storage unit within the block: a word, half, byte or nibble.
 */
struct Tables {
    /** The word of a 32-bit pixel: 8 rows of 8 pixels in a block. */
    std::array<std::array<u8, 8>, 8> column32{};

    /** The half of a 16-bit pixel: 8 rows of 16 pixels in a block. */
    std::array<std::array<u8, 16>, 8> column16{};

    /** The byte of an 8-bit pixel: 16 rows of 16 pixels in a block. */
    std::array<std::array<u8, 16>, 16> column8{};

    /** The nibble of a 4-bit pixel: 16 rows of 32 pixels in a block. */
    std::array<std::array<u16, 32>, 16> column4{};

    /** Fills the four tables from `column_word`. */
    constexpr Tables() {
        // The tables hold 8 rows for the 32-bit and 16-bit formats, which have 8 rows a block.
        for (u32 y = 0; y < 8; y++) {
            // A word of 32 bits: 16 words a column, two rows of the block in each column.
            for (u32 x = 0; x < 8; x++) {
                column32[y][x] = static_cast<u8>((y >> 1) * 16 + column_word(x, y & 1));
            }

            // Two 16-bit pixels share a word; x bit 3 picks the half.
            for (u32 x = 0; x < 16; x++) {
                u32 word = (y >> 1) * 16 + column_word(x & 7, y & 1);
                column16[y][x] = static_cast<u8>(word * 2 + (x >> 3));
            }
        }

        // The 8-bit and 4-bit formats have 16 rows a block, in four columns of four rows.
        for (u32 y = 0; y < 16; y++) {
            u32 column = y >> 2;
            u32 row = y & 3;

            // The second pair of rows (the first, in odd columns) swaps the groups of four words.
            bool swap = ((row >> 1) ^ (column & 1)) != 0;

            // Four 8-bit pixels share a word: x bit 3 and the row's top bit pick the byte.
            for (u32 x = 0; x < 16; x++) {
                u32 x8 = (x & 7) ^ (swap ? 4 : 0);
                u32 word = column * 16 + column_word(x8, row & 1);
                column8[y][x] = static_cast<u8>(word * 4 + (x >> 3) * 2 + (row >> 1));
            }

            // Eight 4-bit pixels share a word: x bits 3-4 and the row's top bit pick the nibble.
            for (u32 x = 0; x < 32; x++) {
                u32 x8 = (x & 7) ^ (swap ? 4 : 0);
                u32 word = column * 16 + column_word(x8, row & 1);
                column4[y][x] = static_cast<u16>(word * 8 + (x >> 3) * 2 + (row >> 1));
            }
        }
    }
};

/** The column tables, built once when the program is compiled. */
constexpr Tables kTables{};

/** Mask that wraps a block number at the end of local memory. */
constexpr u32 kBlockMask = GsMemory::kBlocks - 1;

/**
 * Returns the x half of a block number in a wide page. The x and y halves of the three block
 * orders do not overlap in their bits.
 *
 * @param bx Block column within the page, 0-7.
 * @return The bits of the block number that come from the column.
 */
constexpr u32 wide_x(u32 bx) {
    return (bx & 1) | ((bx & 2) << 1) | ((bx & 4) << 2);
}

/**
 * Returns the y half of a block number in a wide page.
 *
 * @param by Block row within the page, 0-3.
 * @return The bits of the block number that come from the row.
 */
constexpr u32 wide_y(u32 by) {
    return ((by & 1) << 1) | ((by & 2) << 2);
}

/**
 * Returns the x half of a block number in a tall page.
 *
 * @param bx Block column within the page, 0-3.
 * @return The bits of the block number that come from the column.
 */
constexpr u32 tall_x(u32 bx) {
    return ((bx & 1) << 1) | ((bx & 2) << 2);
}

/**
 * Returns the y half of a block number in a tall page.
 *
 * @param by Block row within the page, 0-7.
 * @return The bits of the block number that come from the row.
 */
constexpr u32 tall_y(u32 by) {
    return (by & 1) | ((by & 2) << 1) | ((by & 4) << 2);
}

/**
 * Returns the x half of a block number in a tall page of PSMCT16S and PSMZ16S.
 *
 * @param bx Block column within the page, 0-3.
 * @return The bits of the block number that come from the column.
 */
constexpr u32 tall_s_x(u32 bx) {
    return ((bx & 1) << 1) | ((bx & 2) << 3);
}

/**
 * Returns the y half of a block number in a tall page of PSMCT16S and PSMZ16S.
 *
 * @param by Block row within the page, 0-7.
 * @return The bits of the block number that come from the row.
 */
constexpr u32 tall_s_y(u32 by) {
    return (by & 1) | (by & 4) | ((by & 2) << 2);
}

/** The layouts `make_layout` can build: page shape (W wide, T tall), bits a pixel, Z or not. */
enum class Kind {
    W32,    // PSMCT32 and the formats kept in a 32-bit pixel
    W32Z,   // PSMZ32 and PSMZ24
    T16,    // PSMCT16
    T16Z,   // PSMZ16
    T16S,   // PSMCT16S
    T16SZ,  // PSMZ16S
    W8,     // PSMT8
    T4      // PSMT4
};

/**
 * Builds the address tables of one layout.
 *
 * @param kind Which layout to build.
 * @return The tables, to be combined as the `GsMemory::Layout` comment says.
 */
GsMemory::Layout make_layout(Kind kind) {
    GsMemory::Layout l;

    // Every table has an entry for each coordinate up to 2048, which x and y wrap at.
    for (u32 n = 0; n < 2048; n++) {
        u32 x = n, y = n;
        u32 cwx = (x & 1) | (((x & 7) >> 1) << 2);       // the x part of a word's place in a column
        u32 cwy = ((y & 7) >> 1) * 16 + ((y & 1) << 1);  // and the y part, for the two-row columns

        switch (kind) {
            /*
             * 32-bit pixels: a page is 64 by 32 pixels, 8 by 4 blocks of 8 by 8. The page's first
             * block comes from x / 64 and y / 32 (32 blocks a page), then the block inside it,
             * with the Z form inverting the top two bits (documented).
             */
            case Kind::W32:
            case Kind::W32Z:
                l.unit = 64;
                l.px[n] = static_cast<u16>(
                    ((x >> 1) & ~0x1Fu) + (wide_x((x >> 3) & 7) ^ (kind == Kind::W32Z ? 16u : 0u))
                );
                l.sy[n] = static_cast<u16>(wide_y((y >> 3) & 3) ^ (kind == Kind::W32Z ? 8u : 0u));
                l.rowbase[n] = static_cast<u16>(y & ~0x1Fu);
                l.cx[n] = static_cast<u16>(cwx);
                l.cy[n] = static_cast<u16>(cwy);
                break;

            /*
             * 16-bit pixels: a page is 64 by 64 pixels, 4 by 8 blocks of 16 by 8, and two pixels
             * share a word. The Z form inverts the top two bits of the block number.
             */
            case Kind::T16:
            case Kind::T16Z:
                l.unit = 128;
                l.px[n] = static_cast<u16>(
                    ((x >> 1) & ~0x1Fu) + (tall_x((x >> 4) & 3) ^ (kind == Kind::T16Z ? 8u : 0u))
                );
                l.sy[n] = static_cast<u16>(tall_y((y >> 3) & 7) ^ (kind == Kind::T16Z ? 16u : 0u));
                l.rowbase[n] = static_cast<u16>((y >> 1) & ~0x1Fu);
                l.cx[n] = static_cast<u16>((cwx << 1) | ((x >> 3) & 1));
                l.cy[n] = static_cast<u16>(cwy << 1);
                break;

            // The same as 16-bit pixels, with the block order of PSMCT16S.
            case Kind::T16S:
            case Kind::T16SZ:
                l.unit = 128;
                l.px[n] = static_cast<u16>(
                    ((x >> 1) & ~0x1Fu)
                    + (tall_s_x((x >> 4) & 3) ^ (kind == Kind::T16SZ ? 16u : 0u))
                );
                l.sy[n] =
                    static_cast<u16>(tall_s_y((y >> 3) & 7) ^ (kind == Kind::T16SZ ? 8u : 0u));
                l.rowbase[n] = static_cast<u16>((y >> 1) & ~0x1Fu);
                l.cx[n] = static_cast<u16>((cwx << 1) | ((x >> 3) & 1));
                l.cy[n] = static_cast<u16>(cwy << 1);
                break;

            // 8-bit and 4-bit pixels: pages of 128 by 64 and 128 by 128 pixels (documented).
            case Kind::W8:
            case Kind::T4: {
                /*
                 * Four-row columns: the second pair of rows (the first, in odd columns) swaps the
                 * two groups of four words, which is a flip of one bit that the x part also sets,
                 * hence the exclusive or.
                 */
                u32 column = (y & 15) >> 2, r = y & 3;
                u32 swap = (r >> 1) ^ (column & 1);
                u32 word_y = column * 16 + ((r & 1) << 1);

                // Pages are 128 pixels wide, so `bw` counts two pages for each of its units.
                l.bw_shift = 1;

                // 8-bit pixels: 256 bytes a block, four to a word; the swap is word bit 3.
                if (kind == Kind::W8) {
                    l.unit = 256;
                    l.px[n] = static_cast<u16>(((x >> 2) & ~0x1Fu) + wide_x((x >> 4) & 7));
                    l.sy[n] = static_cast<u16>(wide_y((y >> 4) & 3));
                    l.rowbase[n] = static_cast<u16>((y >> 1) & ~0x1Fu);
                    l.cx[n] = static_cast<u16>((cwx << 2) | (((x >> 3) & 1) << 1));
                    l.cy[n] = static_cast<u16>(((word_y << 2) | (r >> 1)) ^ (swap ? 32u : 0u));
                } else {
                    // 4-bit pixels: 512 nibbles a block, eight to a word.
                    l.unit = 512;
                    l.px[n] = static_cast<u16>(((x >> 2) & ~0x1Fu) + tall_x((x >> 5) & 3));
                    l.sy[n] = static_cast<u16>(tall_y((y >> 4) & 7));
                    l.rowbase[n] = static_cast<u16>((y >> 2) & ~0x1Fu);
                    l.cx[n] = static_cast<u16>((cwx << 3) | (((x >> 3) & 3) << 1));
                    l.cy[n] = static_cast<u16>(((word_y << 3) | (r >> 1)) ^ (swap ? 64u : 0u));
                }
                break;
            }
        }
    }

    return l;
}

}  // namespace

unsigned transfer_bits(u32 psm) {
    switch (psm) {
        case PSMCT32:  // 32-bit colour
        case PSMZ32:
            return 32;

        case PSMCT24:  // 24-bit colour
        case PSMZ24:
            return 24;

        case PSMCT16:  // 16-bit colour
        case PSMCT16S:
        case PSMZ16:
        case PSMZ16S:
            return 16;

        case PSMT8:  // 8-bit index
        case PSMT8H:
            return 8;

        case PSMT4:  // 4-bit index
        case PSMT4HL:
        case PSMT4HH:
            return 4;

        // Not a format: it counts as 32 bits.
        default:
            return 32;
    }
}

GsMemory::GsMemory() : bytes_(kBytes) {}

const GsMemory::Layout& GsMemory::layout(u32 psm) {
    // Built on the first call; the formats that share a layout share a table.
    static const Layout w32 = make_layout(Kind::W32), w32z = make_layout(Kind::W32Z),
                        t16 = make_layout(Kind::T16), t16z = make_layout(Kind::T16Z),
                        t16s = make_layout(Kind::T16S), t16sz = make_layout(Kind::T16SZ),
                        w8 = make_layout(Kind::W8), t4 = make_layout(Kind::T4);

    switch (psm) {
        case PSMCT16:  // 16-bit colour
            return t16;

        case PSMCT16S:  // 16-bit colour, the other block order
            return t16s;

        case PSMT8:  // 8-bit index
            return w8;

        case PSMT4:  // 4-bit index
            return t4;

        case PSMZ32:  // Z buffer, 32 or 24 bits
        case PSMZ24:
            return w32z;

        case PSMZ16:  // Z buffer, 16 bits
            return t16z;

        case PSMZ16S:  // Z buffer, 16 bits, the other block order
            return t16sz;

        default:
            return w32;  // the 32 and 24-bit formats and the ones kept in a 32-bit pixel's top byte
    }
}

u32 GsMemory::address32(u32 bp, u32 bw, u32 x, u32 y, bool z) {
    /*
     * A page is 32 blocks: 64 by 32 pixels, so x / 64 and y / 32 give the page. The Z form
     * inverts the top two bits of the block number within it (documented).
     */
    u32 block = bp + (y & ~0x1Fu) * bw + ((x >> 1) & ~0x1Fu)
                + (block_wide((x >> 3) & 7, (y >> 3) & 3) ^ (z ? 0x18u : 0u));

    // 64 words a block, the word within it from the column table.
    return (block & kBlockMask) * 64 + kTables.column32[y & 7][x & 7];
}

u32 GsMemory::address16(u32 bp, u32 bw, u32 x, u32 y, bool z) {
    // A page is 64 by 64 pixels, so x / 64 and y / 64 give the page.
    u32 block = bp + ((y >> 1) & ~0x1Fu) * bw + ((x >> 1) & ~0x1Fu)
                + (block_tall((x >> 4) & 3, (y >> 3) & 7) ^ (z ? 0x18u : 0u));

    // 128 halves a block.
    return (block & kBlockMask) * 128 + kTables.column16[y & 7][x & 15];
}

u32 GsMemory::address16s(u32 bp, u32 bw, u32 x, u32 y, bool z) {
    // As `address16`, with the block order of PSMCT16S.
    u32 block = bp + ((y >> 1) & ~0x1Fu) * bw + ((x >> 1) & ~0x1Fu)
                + (block_tall_s((x >> 4) & 3, (y >> 3) & 7) ^ (z ? 0x18u : 0u));
    return (block & kBlockMask) * 128 + kTables.column16[y & 7][x & 15];
}

u32 GsMemory::address8(u32 bp, u32 bw, u32 x, u32 y) {
    // A page is 128 by 64 pixels, so a unit of `bw` is half a page.
    u32 block = bp + ((y >> 1) & ~0x1Fu) * (bw >> 1) + ((x >> 2) & ~0x1Fu)
                + block_wide((x >> 4) & 7, (y >> 4) & 3);

    // 256 bytes a block.
    return (block & kBlockMask) * 256 + kTables.column8[y & 15][x & 15];
}

u32 GsMemory::address4(u32 bp, u32 bw, u32 x, u32 y) {
    // A page is 128 by 128 pixels, so a unit of `bw` is half a page.
    u32 block = bp + ((y >> 2) & ~0x1Fu) * (bw >> 1) + ((x >> 2) & ~0x1Fu)
                + block_tall((x >> 5) & 3, (y >> 4) & 7);

    // 512 nibbles a block.
    return (block & kBlockMask) * 512 + kTables.column4[y & 15][x & 31];
}

u32 GsMemory::read(u32 psm, u32 bp, u32 bw, u32 x, u32 y) const {
    switch (psm) {
        case PSMCT32:  // the whole word
            return word(address32(bp, bw, x, y));

        case PSMCT24:  // the low 24 bits of the word
            return word(address32(bp, bw, x, y)) & 0x00FFFFFFu;

        case PSMCT16:
            return half(address16(bp, bw, x, y));

        case PSMCT16S:
            return half(address16s(bp, bw, x, y));

        case PSMT8:
            return bytes_[address8(bp, bw, x, y)];

        case PSMT4: {
            // The low nibble of a byte is the even pixel.
            u32 a = address4(bp, bw, x, y);
            return (bytes_[a >> 1] >> ((a & 1) * 4)) & 0xF;
        }

        case PSMT8H:  // bits 24-31 of a 32-bit pixel
            return word(address32(bp, bw, x, y)) >> 24;

        case PSMT4HL:  // bits 24-27
            return (word(address32(bp, bw, x, y)) >> 24) & 0xF;

        case PSMT4HH:  // bits 28-31
            return word(address32(bp, bw, x, y)) >> 28;

        case PSMZ32:
            return word(address32(bp, bw, x, y, true));

        case PSMZ24:  // the low 24 bits of the word
            return word(address32(bp, bw, x, y, true)) & 0x00FFFFFFu;

        case PSMZ16:
            return half(address16(bp, bw, x, y, true));

        case PSMZ16S:
            return half(address16s(bp, bw, x, y, true));

        // Not a format: reads as 0.
        default:
            return 0;
    }
}

void GsMemory::write(u32 psm, u32 bp, u32 bw, u32 x, u32 y, u32 value) {
    // Stores the bits of `v` that `mask` selects into word `index`, called by the cases below.
    auto merge = [this](u32 index, u32 v, u32 mask) {
        set_word(index, (word(index) & ~mask) | (v & mask));
    };

    switch (psm) {
        case PSMCT32:
            set_word(address32(bp, bw, x, y), value);
            break;

        case PSMCT24:  // the low 24 bits; the top byte stays
            merge(address32(bp, bw, x, y), value, 0x00FFFFFFu);
            break;

        case PSMCT16:
            set_half(address16(bp, bw, x, y), static_cast<u16>(value));
            break;

        case PSMCT16S:
            set_half(address16s(bp, bw, x, y), static_cast<u16>(value));
            break;

        case PSMT8:
            bytes_[address8(bp, bw, x, y)] = static_cast<u8>(value);
            break;

        case PSMT4: {
            // A nibble: the low half of a byte is the even pixel, and the other half stays.
            u32 a = address4(bp, bw, x, y);
            unsigned shift = (a & 1) * 4;
            u8& b = bytes_[a >> 1];
            b = static_cast<u8>((b & ~(0xF << shift)) | ((value & 0xF) << shift));
            break;
        }

        case PSMT8H:  // bits 24-31 of a 32-bit pixel; the rest stays
            merge(address32(bp, bw, x, y), value << 24, 0xFF000000u);
            break;

        case PSMT4HL:  // bits 24-27
            merge(address32(bp, bw, x, y), value << 24, 0x0F000000u);
            break;

        case PSMT4HH:  // bits 28-31
            merge(address32(bp, bw, x, y), value << 28, 0xF0000000u);
            break;

        case PSMZ32:
            set_word(address32(bp, bw, x, y, true), value);
            break;

        case PSMZ24:  // the low 24 bits; the top byte stays
            merge(address32(bp, bw, x, y, true), value, 0x00FFFFFFu);
            break;

        case PSMZ16:
            set_half(address16(bp, bw, x, y, true), static_cast<u16>(value));
            break;

        case PSMZ16S:
            set_half(address16s(bp, bw, x, y, true), static_cast<u16>(value));
            break;

        // Not a format: nothing is written.
        default:
            break;
    }
}

}  // namespace ps2
