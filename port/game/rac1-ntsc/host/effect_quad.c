/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */
/* Carried from port/game/rac1-pal/host/effect_quad.c through the version map (../README.md): names and
 * addresses are this version's. Not yet checked in play on NTSC-U. */
#include "openrac/game_host.h"
#include <string.h>

/*
 * Where an effect texture's pixels and CLUT are, for the TEX0 GetEffectTex (game_FUN_001f44b8) gave it:
 * its upload records this frame (D_0018D040, D_0015F458 of them; +0 the CLUT's address, +6 its
 * block, +8 the pixels' address, +0xE their block), which the game's texture paging turns into GS
 * uploads; a TEX0 seen before keeps the sources it had then (the game caches the word and records
 * the upload only when it pages the texture in).
 */
static void effect_texture_source(uint64_t tex0, uint32_t* pixels, uint32_t* clut) {
    static struct {
        uint64_t tex0;
        uint32_t pixels, clut;
    } known[256];
    static int next = 0;
    const uint32_t tbp = (uint32_t)(tex0 & 0x3FFF);
    const uint32_t cbp = (uint32_t)((tex0 >> 37) & 0x3FFF);
    const int count = GREF(int, OPENRAC_DATA(0x0015F458u));
    const gaddr records = OPENRAC_DATA(0x0018D040u);
    for (int i = 0; i < count && i < 64; ++i) {
        const uint8_t* r = G(records + (gaddr)i * 16);
        if (*(const uint16_t*)(r + 0xE) == tbp && *(const uint16_t*)(r + 6) == cbp) {
            *clut = *(const uint32_t*)(r + 0);
            *pixels = *(const uint32_t*)(r + 8);
            for (int k = 0; k < 256; ++k) {
                if (known[k].tex0 == tex0) {
                    known[k].pixels = *pixels;
                    known[k].clut = *clut;
                    return;
                }
            }
            known[next].tex0 = tex0;
            known[next].pixels = *pixels;
            known[next].clut = *clut;
            next = (next + 1) & 255;
            return;
        }
    }
    for (int k = 0; k < 256; ++k) {
        if (known[k].tex0 == tex0 && tex0 != 0) {
            *pixels = known[k].pixels;
            *clut = known[k].clut;
            return;
        }
    }
}

/*
 * The draw callbacks' quad routine (ReRAC's FastDrawQuadReal): a0 a quad record, four corners
 * (+0x00, x y z w each), four RGBA words (+0x40), four ST pairs (+0x50), then CLAMP_1, TEX0_1,
 * TEX1_1 and ALPHA_1 (+0x70, +0x78, +0x80, +0x88; the GIF packet's register list); a1, when set,
 * a matrix the corners go through first (rows at +0x00..+0x30: x * row0 + y * row1 + z * row2 +
 * w * row3). The game projects the quad with its camera and sends it to the GS as a strip; the
 * window draws it in the world (openrac_game_effect_quad).
 */
void game_FUN_001f7d30(gaddr a0, int a1, int a2) {
    (void)a2;
    const uint8_t* r = G(a0);
    openrac_game_quad q;
    memcpy(q.corner, r, sizeof(q.corner));
    if (a1 != 0) {
        float m[16];
        memcpy(m, G((gaddr)a1), sizeof(m));
        for (int k = 0; k < 4; ++k) {
            const float x = q.corner[k][0], y = q.corner[k][1], z = q.corner[k][2], w = q.corner[k][3];
            for (int i = 0; i < 4; ++i) {
                q.corner[k][i] = x * m[i] + y * m[4 + i] + z * m[8 + i] + w * m[12 + i];
            }
        }
    }
    memcpy(q.rgba, r + 0x40, sizeof(q.rgba));
    memcpy(q.st, r + 0x50, sizeof(q.st));
    memcpy(&q.clamp, r + 0x70, 8);
    memcpy(&q.tex0, r + 0x78, 8);
    memcpy(&q.tex1, r + 0x80, 8);
    memcpy(&q.alpha, r + 0x88, 8);
    q.pixels = 0;
    q.clut = 0;
    effect_texture_source(q.tex0, &q.pixels, &q.clut);
    openrac_game_effect_quad(&q);
}

/* PAL 001FD1D8 and executable 001F7EF8 read the same 0x90-byte
 * record and optional matrix. Camera projection and clipping are performed
 * by the native effect renderer for both entry points. */
void func_L00_001FD228(gaddr a0, gaddr a1, int a2) {
    game_FUN_001f7d30(a0, (int)a1, a2);
}
