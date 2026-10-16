/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (PAL): the liquid grid's strips (func_L02_00250A58, hand-written VU0 assembly in
 * retail, 0x220 bytes), in C.
 *
 * The liquid grid module's users (ReRAC's table: Kerwan, Rilgar's sea (class 879), Umbris, Batalia,
 * Gaspar's lava, Oltanis) draw it from a draw callback: func_L03_00292AC0 sets the fog and the animated image,
 * sends the strip order (124 shorts) to the scratchpad at 0x70002800, culls the grid's 7 x 7-cell
 * blocks (func_L02_00250720: a flag byte per block at 0x70002000, 0 for a block not drawn, else
 * the VU1 entry's variant), then calls this. Written from the instructions, with ReRAC's notes on
 * it (docs/plan/world_animation.md section 3.3, "the liquid grid module"; ISC License, Copyright
 * (c) 2026 ReRAC contributors).
 *
 * The grid record (rec): +0x00 the grid's corner (x, y, and z, the liquid's height), +0x10 a
 * cell's size (x, y), +0x18 / +0x1A the grid's columns and rows (cells), +0x34 the blocks'
 * colours: 0x200 bytes a block, an unpack header then 124 RGBA words. Per drawn block, retail
 * puts the block's 8 x 8 grid points (x, y floats) in the scratchpad, then builds one packet: the
 * 124 positions (the points the strip order names, at the liquid's height), a reference to the
 * block's colours and one to the shared texture coordinates (D_L02_001CBC40: an unpack header,
 * then 124 ST pairs), and a call of the water strip VU1 program at entry 12 (flag 1) or 14.
 *
 * The port draws the strips the VU1 program would from that program's packet as
 * func_L00_001FDE48 builds it, the same program and entries: here each block's strip goes through
 * it, with the same positions, colours, texture coordinates and entry. Without this function
 * (it had no C) the seas were never drawn and only their faint top layers showed: the world was
 * see-through where the water should be.
 *
 * Refactor notes: the record wants a struct (rac1-decomp has none yet); the scratchpad grid is
 * kept as retail fills it, so the strip order's byte offsets index it unchanged.
 */
#include "common.h"

extern unsigned char D_L02_001CBC40[];  /* the blocks' shared texture coordinates */
extern void func_L00_001FDE48(int n, unsigned char *xyz, unsigned char *colours, unsigned char *uvs, int flag);

#define LIQUID_STRIP 124

/*
 * func_L02_002100E8(from, to, quadwords): a transfer into the scratchpad on the toSPR DMA channel
 * (0x1000D400: SADR, MADR, QWC, then CHCR started), retail's 0x30 bytes of assembly. The port has
 * no DMA: the copy is done at once (the channel is idle by the time anything reads the scratchpad).
 * The liquid grid sends its strip order to 0x70002800 with it.
 */
void func_L02_002100E8(unsigned char *from, unsigned int to, int quadwords) {
    unsigned char *out = (unsigned char *)(0x70000000u | (to & 0x3FF0u));
    int i;
    for (i = 0; i < quadwords * 16; i++) {
        out[i] = from[i];
    }
}

void func_L02_00250A58(unsigned char *rec) {
    unsigned char *flags = (unsigned char *)0x70002000;      /* the cull's flag per block */
    const short *order = (const short *)0x70002800;          /* the strip's grid points, as byte offsets */
    float *grid = (float *)0x70000000;                       /* the block's 8 x 8 points, x y each */
    const float x0 = *(float *)(rec + 0x00);
    const float y0 = *(float *)(rec + 0x04);
    const float z = *(float *)(rec + 0x08);
    const float cx = *(float *)(rec + 0x10);
    const float cy = *(float *)(rec + 0x14);
    unsigned char *colours = *(unsigned char **)(rec + 0x34);
    float xyz[LIQUID_STRIP * 3];
    float by = y0;
    int rows = *(unsigned short *)(rec + 0x1A);
    int r, c, k;

    for (; rows > 0; rows -= 7, by += cy * 7.0f) {
        int cols = *(unsigned short *)(rec + 0x18);
        float bx = x0;
        for (; cols > 0; cols -= 7, bx += cx * 7.0f, colours += 0x200) {
            const int flag = *flags++;
            if (flag == 0) {
                continue;
            }
            for (r = 0; r < 8; r++) {
                for (c = 0; c < 8; c++) {
                    grid[(r * 8 + c) * 2] = bx + cx * (float)c;
                    grid[(r * 8 + c) * 2 + 1] = by + cy * (float)r;
                }
            }
            for (k = 0; k < LIQUID_STRIP; k++) {
                const float *p = (const float *)((unsigned char *)grid + order[k]);
                xyz[k * 3] = p[0];
                xyz[k * 3 + 1] = p[1];
                xyz[k * 3 + 2] = z;
            }
            /* Retail's entry is 12 + (flag - 1) * 2; func_L00_001FDE48 takes 14 for flag bit 0. */
            func_L00_001FDE48(LIQUID_STRIP, (unsigned char *)xyz, colours + 0x10, D_L02_001CBC40 + 0x10,
                              flag == 2 ? 1 : 0);
        }
    }
}
