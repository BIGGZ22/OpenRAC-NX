/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The graphics and DMA libraries (libgraph, libdma): where the game hands its
 * drawing to the hardware, the port hands it to its renderer instead.
 *
 * The environment structures keep the GS register values the game builds,
 * in the layout the game itself reads and writes (64-bit register values,
 * each followed by its register address where the structure is a GIF
 * packet), so code that changes one field directly still works. The port
 * decodes them; it never sends them anywhere. */
#include "openrac/game_host.h"
#include "openrac/game_lib.h"

/* GS register values, built from their fields. */
static uint64_t bits(uint64_t v, int at, int width) {
    return (v & ((1ull << width) - 1)) << at;
}

/* sceGsResetGraph(mode, interlace, video mode, field mode) */
void openrac_lib_sceGsResetGraph(short mode, short interlace, short video, short field) {
    (void)mode;
    openrac_game_set_video_mode(interlace, video, field);
}

/* sceGsSetDefDispEnv(disp, psm, width, height, dx, dy): the display
 * environment is {PMODE, SMODE2, DISPFB, DISPLAY, BGCOLOR}, 64 bits each. */
void openrac_lib_sceGsSetDefDispEnv(gaddr disp, short psm, short w, short h, short dx, short dy) {
    uint64_t* d = (uint64_t*)G(disp);
    const int magh = 2560 / (w > 0 ? w : 640) - 1; /* the console's horizontal clock divider */
    d[0] = bits(1, 0, 1) | bits(1, 5, 1) | bits(0xFF, 8, 8); /* PMODE: circuit 1, fixed alpha */
    d[1] = bits(1, 0, 1) | bits(1, 1, 1);                    /* SMODE2: interlaced, frame */
    d[2] = bits(0, 0, 9) | bits((uint64_t)(w + 63) / 64, 9, 6)
           | bits((uint64_t)psm, 15, 5); /* DISPFB */
    d[3] = bits((uint64_t)(636 + dx * (magh + 1)), 0, 12) | bits((uint64_t)(50 + dy), 12, 11)
           | bits((uint64_t)magh, 23, 4) | bits((uint64_t)((w * (magh + 1)) - 1), 32, 12)
           | bits((uint64_t)(h - 1), 44, 11); /* DISPLAY */
    d[4] = 0;                                 /* BGCOLOR */
}

/* sceGsPutDispEnv: the frame buffer to show. */
void openrac_lib_sceGsPutDispEnv(gaddr disp) {
    const uint64_t* d = (const uint64_t*)G(disp);
    const uint64_t dispfb = d[2], display = d[3];
    const int magh = (int)((display >> 23) & 0xF) + 1;
    openrac_game_display out;
    out.frame_base = (int)(dispfb & 0x1FF);
    out.frame_width = (int)((dispfb >> 9) & 0x3F);
    out.psm = (int)((dispfb >> 15) & 0x1F);
    out.width = (int)(((display >> 32) & 0xFFF) + 1) / magh;
    out.height = (int)((display >> 44) & 0x7FF) + 1;
    out.x = (int)(dispfb >> 32 & 0x7FF);
    out.y = (int)(dispfb >> 43 & 0x7FF);
    openrac_game_set_display(&out);
}

/* sceGsPutDrawEnv: the drawing environment is a GIF packet of register
 * writes; the renderer takes the same writes from the display list, so
 * there is nothing more to do here. */
int openrac_lib_sceGsPutDrawEnv(gaddr draw) {
    (void)draw;
    return 0;
}

/* sceGsSyncV: the end of a frame. */
int openrac_lib_sceGsSyncV(int mode) {
    (void)mode;
    const int field = openrac_game_vsync();
    openrac_game_run_vsync_handlers();
    return field;
}

/* sceGsResetPath, sceGsSyncPath: nothing is in flight. */
void openrac_lib_sceGsResetPath(void) {}

int openrac_lib_sceGsSyncPath(int mode, unsigned short timeout) {
    (void)mode;
    (void)timeout;
    return 0;
}

/* An image transfer, as the game keeps it: a GIF tag, then BITBLTBUF,
 * TRXPOS, TRXREG and TRXDIR as (value, register) pairs, then the GIF tag of
 * the image data. 96 bytes. */
typedef struct {
    uint64_t tag[2];
    uint64_t bitbltbuf, bitbltbuf_reg;
    uint64_t trxpos, trxpos_reg;
    uint64_t trxreg, trxreg_reg;
    uint64_t trxdir, trxdir_reg;
    uint64_t image_tag[2];
} image_transfer;

static void set_transfer(
    gaddr at, int dir, short base, short width, short psm, short x, short y, short w, short h
) {
    image_transfer* t = (image_transfer*)G(at);
    t->tag[0] = bits(4, 0, 15) | bits(1, 15, 1) | bits(0, 58, 2)
                | bits(1, 60, 4); /* NLOOP 4, EOP, PACKED, 1 reg */
    t->tag[1] = 0xE;              /* A+D */
    if (dir == 0) {               /* host to GS: destination fields */
        t->bitbltbuf = bits((uint64_t)base, 32, 14) | bits((uint64_t)width, 48, 6)
                       | bits((uint64_t)psm, 56, 6);
        t->trxpos = bits((uint64_t)x, 32, 11) | bits((uint64_t)y, 48, 11);
    } else { /* GS to host: source fields */
        t->bitbltbuf =
            bits((uint64_t)base, 0, 14) | bits((uint64_t)width, 16, 6) | bits((uint64_t)psm, 24, 6);
        t->trxpos = bits((uint64_t)x, 0, 11) | bits((uint64_t)y, 16, 11);
    }
    t->bitbltbuf_reg = 0x50;
    t->trxpos_reg = 0x51;
    t->trxreg = bits((uint64_t)w, 0, 12) | bits((uint64_t)h, 32, 12);
    t->trxreg_reg = 0x52;
    t->trxdir = (uint64_t)dir;
    t->trxdir_reg = 0x53;
    /* IMAGE, with the rectangle's size in quadwords as NLOOP: the game also sends a transfer
     * built here in its own display list (the pause menu's frame snapshot, 64 x 64 strips), and
     * the GS then takes exactly that much image data after the tag. */
    if (dir == 0) {
        const int bpp = psm == 0x00 ? 32 : psm == 0x01 ? 24 : (psm == 0x02 || psm == 0x0A) ? 16
                        : psm == 0x13 ? 8 : psm == 0x14 ? 4 : 32;
        const uint64_t qwords = ((uint64_t)w * (uint64_t)h * (uint64_t)bpp / 8 + 15) / 16;
        t->image_tag[0] = bits(qwords > 0x7FFF ? 0x7FFF : qwords, 0, 15) | bits(1, 15, 1) | bits(2, 58, 2);
    } else {
        t->image_tag[0] = bits(2, 58, 2); /* IMAGE; nothing follows a GS-to-host transfer's tag */
    }
    t->image_tag[1] = 0;
}

/* sceGsSetDefLoadImage(transfer, base, width, psm, x, y, w, h) */
void openrac_lib_sceGsSetDefLoadImage(
    gaddr transfer, short base, short width, short psm, short x, short y, short w, short h
) {
    set_transfer(transfer, 0, base, width, psm, x, y, w, h);
}

/* sceGsSetDefStoreImage(transfer, base, width, psm, x, y, w, h) */
void openrac_lib_sceGsSetDefStoreImage(
    gaddr transfer, short base, short width, short psm, short x, short y, short w, short h
) {
    set_transfer(transfer, 1, base, width, psm, x, y, w, h);
}

/* sceGsExecLoadImage(transfer, pixels): to the renderer's textures. */
int openrac_lib_sceGsExecLoadImage(gaddr transfer, gaddr pixels) {
    const image_transfer* t = (const image_transfer*)G(transfer);
    openrac_game_image image;
    image.base = (int)((t->bitbltbuf >> 32) & 0x3FFF);
    image.width_units = (int)((t->bitbltbuf >> 48) & 0x3F);
    image.psm = (int)((t->bitbltbuf >> 56) & 0x3F);
    image.x = (int)((t->trxpos >> 32) & 0x7FF);
    image.y = (int)((t->trxpos >> 48) & 0x7FF);
    image.width = (int)(t->trxreg & 0xFFF);
    image.height = (int)((t->trxreg >> 32) & 0xFFF);
    image.pixels = pixels;
    openrac_game_load_image(&image);
    return 0;
}

/* sceGsExecStoreImage(transfer, destination): reading the GS's memory back.
 * The renderer keeps no GS memory; it answers what it can (the frame buffer,
 * which the pause and vendor menus read to draw themselves over). */
int openrac_lib_sceGsExecStoreImage(gaddr transfer, gaddr destination) {
    const image_transfer* t = (const image_transfer*)G(transfer);
    openrac_game_image image;
    image.base = (int)(t->bitbltbuf & 0x3FFF);
    image.width_units = (int)((t->bitbltbuf >> 16) & 0x3F);
    image.psm = (int)((t->bitbltbuf >> 24) & 0x3F);
    image.x = (int)(t->trxpos & 0x7FF);
    image.y = (int)((t->trxpos >> 16) & 0x7FF);
    image.width = (int)(t->trxreg & 0xFFF);
    image.height = (int)((t->trxreg >> 32) & 0xFFF);
    image.pixels = destination;
    openrac_game_store_image(&image);
    return 0;
}

/* sceDmaReset */
int openrac_lib_sceDmaReset(int mode) {
    (void)mode;
    return 0;
}

/* sceDmaSend(channel, chain): replace the SDK's MMIO programming with a
 * synchronous renderer hand-off and then deliver the channel's completion
 * interrupt through the game's registered handler. */
void openrac_lib_sceDmaSend(gaddr channel, gaddr tag) {
    /* libdma translates the 0x7... segment to its cached 0x8... alias before
     * writing TADR. Preserve that address convention for DMA chains. */
    if ((tag >> 28) == 7u) {
        tag = (tag & 0x0FFFFFFFu) | 0x80000000u;
    }

    uint32_t tadr;
    memcpy(&tadr, G(channel + 0x30u), sizeof tadr);
    if (tadr != 0xFFFFFFFFu) {
        memcpy(G(channel + 0x30u), &tag, sizeof tag);
    }
    const uint32_t qwc = 0;
    memcpy(G(channel + 0x20u), &qwc, sizeof qwc);

    uint32_t chcr;
    memcpy(&chcr, G(channel), sizeof chcr);
    chcr = (chcr & ~0xCu) | 0x105u;
    memcpy(G(channel), &chcr, sizeof chcr);

    openrac_game_dma_send(channel, tag);

    /* The renderer has consumed the chain synchronously. Clear the start
     * bit before notifying the game so polling and completion callbacks both
     * observe an idle channel. Keep any fields changed during the hand-off. */
    memcpy(&chcr, G(channel), sizeof chcr);
    chcr &= ~0x100u;

    /* The DMAC channel number from its register block: 0x10008000 is VIF0
     * (0), 0x10009000 VIF1 (1), 0x1000A000 GIF (2), and so on. */
    const int channel_id = (int)((channel >> 12) & 0xF) - 8;
    if (channel == 0x10009000u) {
        /* VIF1 sends complete synchronously in the native renderer. Present
         * the completion bits that the console DMAC would set before raising
         * channel 1's interrupt; the game's own handler clears its wait flag. */
        chcr = (chcr & ~0x03FF0000u) | 0x02010000u;
    }
    memcpy(G(channel), &chcr, sizeof chcr);
    openrac_game_run_dmac_handlers(channel_id);
}
