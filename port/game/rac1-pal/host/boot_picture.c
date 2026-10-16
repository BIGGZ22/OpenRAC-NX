/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The boot pictures (the memory card warning before the title, the loading pictures): the game
 * copies a raw picture from the boot lump straight into the frame buffer it draws into
 * (libraries.tsv, port = wrap). What these pictures are and where they come from follows ReRAC's
 * notes (ISC License; rc-formats frontend.rs, BootPictures, and docs/plan/progression.md,
 * "Boot -> title"). */
#include "game_protos.h"
#include "openrac/game_host.h"

#include <stdint.h>

/* The display list's write cursor, the PAL flag (448 lines, else 416) and the screen's height in
 * lines (the second word of the viewport at 0x13E600). */
#define DISPLAY_LIST 0x00161000u
#define PAL_LINES 0x0015EE80u
#define SCREEN_LINES 0x0013E604u

/* Base pointers the picture is sent to as textures, one per band of rows. The port's texture pool
 * keeps uploads by base pointer and models no chip memory (renderer/texture_pool.h), so these are
 * names, not room: nothing uploads there before the title, and what the game uploads there later
 * replaces them. */
#define PICTURE_BASE 0x3E00u
#define BAND_ROWS 128

static void put_u32(gaddr *cursor, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    GREF(uint32_t, *cursor + 0) = a;
    GREF(uint32_t, *cursor + 4) = b;
    GREF(uint32_t, *cursor + 8) = c;
    GREF(uint32_t, *cursor + 12) = d;
    *cursor += 16;
}

static void put_u64(gaddr *cursor, uint64_t a, uint64_t b) {
    GREF(uint64_t, *cursor + 0) = a;
    GREF(uint64_t, *cursor + 8) = b;
    *cursor += 16;
}

/*
 * The game sends a 512-pixel-wide PSMCT32 picture at `image` to the draw buffer in bands of 128
 * rows with GIF IMAGE transfers (the DMA reads the pixels where they are). The chip then holds the
 * picture in the frame, which the game fades and copies to the display buffer like any frame.
 *
 * The port's 2D path keeps every image transfer as a texture and draws only primitives into its
 * frame, so a transfer into the frame buffer never shows. After the game's own packets, this sends
 * each band again as a texture of its own and draws it where the transfer put it (row y of the
 * draw buffer) with the game's textured-sprite routine, opaque: what the chip holds after the
 * transfer.
 */
static void draw_picture(gaddr image, int rows) {
    gaddr source = image;
    for (int y = 0, band = 0; y < rows; y += BAND_ROWS, ++band) {
        const int count = rows - y < BAND_ROWS ? rows - y : BAND_ROWS;
        const uint32_t base = PICTURE_BASE + (uint32_t)band;
        const uint32_t quadwords = (uint32_t)count * 512u * 4u / 16u;

        /* BITBLTBUF (512 wide, PSMCT32), TRXPOS 0,0, TRXREG, TRXDIR host to local; then the pixels. */
        gaddr p = GREF(gaddr, OPENRAC_DATA(DISPLAY_LIST));
        put_u32(&p, 0x10000006u, 0, 0, 0x50000006u);
        put_u64(&p, 0x4000000000000001ull, 0xEEEEull);
        put_u64(&p, ((uint64_t)base << 32) | (8ull << 48), 0x50);
        put_u64(&p, 0, 0x51);
        put_u64(&p, ((uint64_t)count << 32) | 512u, 0x52);
        put_u64(&p, 0, 0x53);
        put_u64(&p, 0x0800000000008000ull | quadwords, 0);
        put_u32(&p, 0x30000000u | quadwords, source, 0, 0x50000000u | quadwords);
        GREF(gaddr, OPENRAC_DATA(DISPLAY_LIST)) = p;
        source += (gaddr)count * 512u * 4u;

        /* TEX0: that base, 512 wide, PSMCT32, 512 x 2^th, RGB only (the vertex alpha 0x80 makes the
         * sprite opaque under the routine's blend), modulated by 0x80 (the pixels as they are). */
        int th = 0;
        while ((1 << th) < count) {
            ++th;
        }
        const uint64_t tex0 = (uint64_t)base | (8ull << 14) | (9ull << 26) | ((uint64_t)th << 30);
        func_001F5800(0, y, 512, count, 0, 0, 512, count, 0x80808080ull, tex0);
    }
}

/* draw_bootImage (func_00201AF0, matched C in the decompilation): the picture shown while a level
 * loads, 448 lines on PAL (416 otherwise), then a TEXFLUSH. */
void func_00201AF0(unsigned int image) {
    func_00201AF0__game(image);
    draw_picture(image, GREF(int32_t, OPENRAC_DATA(PAL_LINES)) ? 448 : 416);
}

/* func_001F7680 (the decompilation's functional C): the same transfer of a screen-high picture,
 * every frame of the boot loop (the memory card warning, the loading pictures), then a
 * texture flush (VU1_texFlush). */
void func_001F7680(unsigned int image) {
    func_001F7680__game(image);
    draw_picture((gaddr)image, GREF(int32_t, OPENRAC_DATA(SCREEN_LINES)));
}
