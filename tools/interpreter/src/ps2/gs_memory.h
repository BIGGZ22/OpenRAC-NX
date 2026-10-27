// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The Graphics Synthesizer's local memory and the way every pixel format lays its pixels out in it.
 *
 * The memory is 4 MB of bytes. A format decides which byte, half word or nibble holds the pixel
 * (x, y) of a buffer, so reading and writing go through the address functions here. This file
 * holds the pixel format numbers, the address functions and the same addresses as tables for
 * inner loops. It leaves out the GS's registers and drawing, which are in gs.h.
 *
 * Sources: the GS's local memory organisation (pages, blocks, columns) as publicly documented.
 */

#pragma once

#include <array>
#include <vector>

#include "types.h"

namespace ps2 {

/** Pixel storage formats (the PSM field of FRAME, ZBUF, TEX0 and BITBLTBUF). */
enum : u32 {
    PSMCT32 = 0x00,
    PSMCT24 = 0x01,
    PSMCT16 = 0x02,
    PSMCT16S = 0x0A,
    PSMT8 = 0x13,
    PSMT4 = 0x14,
    PSMT8H = 0x1B,
    PSMT4HL = 0x24,
    PSMT4HH = 0x2C,
    PSMZ32 = 0x30,
    PSMZ24 = 0x31,
    PSMZ16 = 0x32,
    PSMZ16S = 0x3A,
};

/**
 * Returns how many bits one pixel of a format takes in a transfer.
 *
 * @param psm A pixel storage format, one of the PSM values above.
 * @return The bits per pixel on the bus, 32 for a value that is no format.
 */
unsigned transfer_bits(u32 psm);

/**
 * GS local memory: 4 MB, addressed by the games in blocks of 256 bytes. A buffer is a block pointer
 * `bp` and a width `bw` in units of 64 pixels. Where the pixel (x, y) of a buffer lies depends on
 * the format: a page of 8,192 bytes holds 32 blocks in a format-specific order, and a block holds
 * four columns whose pixels are interleaved.
 */
class GsMemory {
public:
    /** Size of local memory in bytes: 4 MB (documented). */
    static constexpr u32 kBytes = 4 * 1024 * 1024;

    /** Number of blocks in local memory; a block is 256 bytes (documented). */
    static constexpr u32 kBlocks = kBytes / 256;

    /** Allocates the memory, zero-filled. */
    GsMemory();

    /**
     * Reads the stored value of a pixel, as wide as the format (32, 24, 16, 8 or 4 bits).
     *
     * PSMT8H, PSMT4HL and PSMT4HH read their bits out of a 32-bit word.
     *
     * @param psm The pixel storage format of the buffer.
     * @param bp The buffer's block pointer, in blocks.
     * @param bw The buffer's width, in units of 64 pixels.
     * @param x Pixel column.
     * @param y Pixel row.
     * @return The stored value, or 0 for a value of `psm` that is no format.
     */
    u32 read(u32 psm, u32 bp, u32 bw, u32 x, u32 y) const;

    /**
     * Writes the stored value of a pixel, keeping the bits of its word that the format leaves.
     *
     * @param psm The pixel storage format of the buffer; a value that is no format writes nothing.
     * @param bp The buffer's block pointer, in blocks.
     * @param bw The buffer's width, in units of 64 pixels.
     * @param x Pixel column.
     * @param y Pixel row.
     * @param value The value to store; the bits beyond the format's width are dropped.
     */
    void write(u32 psm, u32 bp, u32 bw, u32 x, u32 y, u32 value);

    /**
     * Index of the pixel's storage unit: a 32-bit word, a 16-bit half, a byte or a nibble of local
     * memory. Exposed for tests.
     *
     * `address32` serves PSMCT32, PSMCT24, PSMZ32, PSMZ24 and the formats kept in a 32-bit pixel.
     * `address16` serves PSMCT16 and PSMZ16, `address16s` PSMCT16S and PSMZ16S, `address8` PSMT8
     * and `address4` PSMT4.
     *
     * @param bp The buffer's block pointer, in blocks.
     * @param bw The buffer's width, in units of 64 pixels.
     * @param x Pixel column.
     * @param y Pixel row.
     * @param z True for the Z formats, whose block order has its top two bits inverted.
     * @return The index of the unit, counted in units from the start of local memory.
     */
    static u32 address32(u32 bp, u32 bw, u32 x, u32 y, bool z = false);

    /** @copydoc address32 */
    static u32 address16(u32 bp, u32 bw, u32 x, u32 y, bool z = false);

    /** @copydoc address32 */
    static u32 address16s(u32 bp, u32 bw, u32 x, u32 y, bool z = false);

    /**
     * The same as `address32` for a byte pixel; there is no Z form of it.
     *
     * @param bp The buffer's block pointer, in blocks.
     * @param bw The buffer's width, in units of 64 pixels.
     * @param x Pixel column.
     * @param y Pixel row.
     * @return The index of the byte.
     */
    static u32 address8(u32 bp, u32 bw, u32 x, u32 y);

    /**
     * The same as `address32` for a nibble pixel; there is no Z form of it.
     *
     * @param bp The buffer's block pointer, in blocks.
     * @param bw The buffer's width, in units of 64 pixels.
     * @param x Pixel column.
     * @param y Pixel row.
     * @return The index of the nibble.
     */
    static u32 address4(u32 bp, u32 bw, u32 x, u32 y);

    /**
     * The same addresses as tables, for inner loops. A pixel's block is
     * `bp + rowbase[y] * (bw >> bw_shift) + sy[y] + px[x]` and its place in the block is
     * `cx[x] ^ cy[y]`; a Row holds the parts that depend on y.
     */
    struct Layout {
        /**
         * Tables indexed by x or y, which wrap at 2048: the block offset from x (`px`), the place
         * in a block from x (`cx`), the block offset from y (`sy`), the place in a block from y
         * (`cy`), and the first block of the page row from y (`rowbase`).
         */
        std::array<u16, 2048> px{}, cx{}, sy{}, cy{}, rowbase{};

        /** Shift that turns `bw` into pages across: 0 for pages 64 pixels wide, 1 for 128. */
        unsigned bw_shift = 0;

        /** Storage units in a block: 64 words, 128 halves, 256 bytes, 512 nibbles. */
        u32 unit = 64;
    };

    /** The parts of a pixel's address that depend on its row, so a loop along x finds them once. */
    struct Row {
        /**
         * The block of the row's first pixel, before the x offset (`block`), and the row's part of
         * the pixel's place in its block (`in_block`).
         */
        u32 block = 0, in_block = 0;

        /** The tables the row was made from. */
        const Layout* layout = nullptr;
    };

    /**
     * Returns the address tables of a format.
     *
     * @param psm A pixel storage format; a value that is no format gets the PSMCT32 tables.
     * @return A table set built on first use and kept for the life of the program.
     */
    static const Layout& layout(u32 psm);

    /**
     * Computes the row part of the address of every pixel on row `y`.
     *
     * @param l The tables of the buffer's format.
     * @param bp The buffer's block pointer, in blocks.
     * @param bw The buffer's width, in units of 64 pixels.
     * @param y Pixel row; taken modulo 2048.
     * @return The row, to give to `index` with each x.
     */
    static Row row(const Layout& l, u32 bp, u32 bw, u32 y) {
        // The tables hold 2048 entries.
        y &= 2047;
        return {bp + l.rowbase[y] * (bw >> l.bw_shift) + l.sy[y], l.cy[y], &l};
    }

    /**
     * Finds the storage unit of the pixel at column `x` of a row.
     *
     * @param r A row from `row`.
     * @param x Pixel column; taken modulo 2048.
     * @return The index of the unit, the same as the address functions give.
     */
    static u32 index(const Row& r, u32 x) {
        // The tables hold 2048 entries.
        x &= 2047;
        return ((r.block + r.layout->px[x]) & (kBlocks - 1)) * r.layout->unit
               + (r.layout->cx[x] ^ r.in_block);
    }

    /**
     * Finds the storage unit of a pixel from the tables. Forwards to the form with a Row.
     *
     * @param l The tables of the buffer's format.
     * @param bp The buffer's block pointer, in blocks.
     * @param bw The buffer's width, in units of 64 pixels.
     * @param x Pixel column.
     * @param y Pixel row.
     * @return The index of the unit.
     */
    static u32 index(const Layout& l, u32 bp, u32 bw, u32 x, u32 y) {
        return index(row(l, bp, bw, y), x);
    }

    /**
     * Reads a 32-bit word by index, as `index` gives them.
     *
     * @param i Index counted in words; wraps at the end of local memory.
     * @return The word.
     */
    u32 word(u32 i) const { return load<u32>(&bytes_[(i & (kBytes / 4 - 1)) * 4]); }

    /**
     * Writes a 32-bit word by index.
     *
     * @param i Index counted in words; wraps at the end of local memory.
     * @param v The word to store.
     */
    void set_word(u32 i, u32 v) { store<u32>(&bytes_[(i & (kBytes / 4 - 1)) * 4], v); }

    /**
     * Reads a 16-bit half by index.
     *
     * @param i Index counted in halves; wraps at the end of local memory.
     * @return The half.
     */
    u16 half(u32 i) const { return load<u16>(&bytes_[(i & (kBytes / 2 - 1)) * 2]); }

    /**
     * Writes a 16-bit half by index.
     *
     * @param i Index counted in halves; wraps at the end of local memory.
     * @param v The half to store.
     */
    void set_half(u32 i, u16 v) { store<u16>(&bytes_[(i & (kBytes / 2 - 1)) * 2], v); }

    /**
     * Reads a byte by index.
     *
     * @param i Index counted in bytes; wraps at the end of local memory.
     * @return The byte.
     */
    u8 byte(u32 i) const { return bytes_[i & (kBytes - 1)]; }

    /**
     * Reads a nibble by index; the low nibble of a byte comes first.
     *
     * @param i Index counted in nibbles; wraps at the end of local memory.
     * @return The nibble, 0-15.
     */
    u8 nibble(u32 i) const { return (bytes_[(i >> 1) & (kBytes - 1)] >> ((i & 1) * 4)) & 0xF; }

    /**
     * Gives direct access to the bytes of local memory.
     *
     * @return A pointer to the first byte; the memory is `kBytes` long.
     */
    u8* data() { return bytes_.data(); }

    /** The same as `data`, for a memory that is not to be changed. */
    const u8* data() const { return bytes_.data(); }

private:
    /** Local memory. */
    std::vector<u8> bytes_;
};

}  // namespace ps2
