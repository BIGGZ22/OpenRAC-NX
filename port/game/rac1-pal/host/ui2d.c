/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (PAL): the 2D layer's draw routines, wrapped (libraries.tsv, port = wrap) on the
 * way to drawing the HUD and menus natively instead of replaying their GS packets (OPENRAC_UI_REPLAY
 * replays them, for comparing).
 * OPENRAC_UI_LOG: each distinct caller and texture of DrawTexturedQuad, once, in the log.
 */
#include "game_protos.h"
#include "openrac/game_host.h"

#include <execinfo.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void log_caller(const char* what, unsigned long long tex, int w, int h) {
    static const char* seen[256];
    static int count = 0;
    void* frames[3];
    if (backtrace(frames, 3) < 3) {
        return;
    }
    char** names = backtrace_symbols(frames + 2, 1);
    if (names == NULL) {
        return;
    }
    char key[256];
    const char* name = strrchr(names[0], ' ');
    snprintf(key, sizeof(key), "%s%s tbp %#x %dx%d", what, names[0], (unsigned)(tex >> 0) & 0x3FFF, w, h);
    (void)name;
    for (int i = 0; i < count; ++i) {
        if (strcmp(seen[i], key) == 0) {
            free(names);
            return;
        }
    }
    if (count < 256) {
        seen[count++] = strdup(key);
        fprintf(stderr, "ui2d: %s\n", key);
    }
    free(names);
}

/* Inside FontPrint: the quads drawn are glyphs of text. */
static int g_glyphs = 0;

/* FontPrint: a string at (x, y) in colour, glyph by glyph through DrawTexturedQuad. */
void func_001F6668(int x, int y, unsigned long long colour, gaddr text, int limit, long long texture,
                   gaddr glyphs) {
    ++g_glyphs;
    func_001F6668__game(x, y, colour, text, limit, texture, glyphs);
    --g_glyphs;
}

/* DrawTexturedQuad: a w x h screen rectangle at (x, y) textured from (u, v) .. (u + uw, v + vh) of
 * the texture tex (TEX0) in colour rgba: one GS triangle strip appended to the display list. */
void func_001F5800(int x, int y, int w, int h, int u, int v, int uw, int vh, unsigned long long rgba,
                   unsigned long long tex) {
    static int log = -1;
    if (log < 0) {
        log = getenv("OPENRAC_UI_LOG") != NULL;
    }
    if (log) {
        log_caller("quad ", tex, w, h);
    }
    /* The game's own corners: 12.4, the screen offset (D_0013E600 +0x10/+0x14) and half a pixel. */
    const int ofx = GREF(int, 0x0013E610u), ofy = GREF(int, 0x0013E614u);
    openrac_game_ui_rect q;
    q.x0 = (uint32_t)(x * 16 + ofx - 8);
    q.x1 = (uint32_t)((x + w) * 16 + ofx - 8);
    q.y0 = (uint32_t)(y * 16 + ofy - 8);
    q.y1 = (uint32_t)((y + h) * 16 + ofy - 8);
    q.s0 = (uint32_t)(u * 16);
    q.s1 = (uint32_t)((u + uw) * 16);
    q.t0 = (uint32_t)v;
    q.t1 = (uint32_t)(v + vh);
    q.rgba = (uint32_t)rgba;
    q.tex0 = tex;
    q.flags = g_glyphs > 0 ? 1u : 0u;
    const int index = openrac_game_ui_quad(&q);
    if (index < 0) {
        func_001F5800__game(x, y, w, h, u, v, uw, vh, rgba, tex);
        return;
    }
    /* Its marker in the display list instead of its GS packet: DMA cnt and VIF DIRECT of two
     * quadwords, a PACKED GIF tag of one A+D register, and the A+D write to NOP carrying the index. */
    const gaddr at = GREF(gaddr, OPENRAC_DATA(0x00161000u));
    GREF(uint32_t, at + 0) = 0x10000002u;
    GREF(uint32_t, at + 4) = 0;
    GREF(uint32_t, at + 8) = 0;
    GREF(uint32_t, at + 12) = 0x50000002u;
    GREF(uint64_t, at + 16) = 1ull | (1ull << 15) | (1ull << 60);
    GREF(uint64_t, at + 24) = 0xEull;
    GREF(uint64_t, at + 32) = (0x4E55ull << 48) | (uint32_t)index;
    GREF(uint64_t, at + 40) = 0x7Full;
    GREF(gaddr, OPENRAC_DATA(0x00161000u)) = at + 48;
}
