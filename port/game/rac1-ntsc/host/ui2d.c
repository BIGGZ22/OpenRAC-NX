/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (NTSC-U): the 2D layer's draw routines, wrapped (libraries.tsv, port = wrap) so the
 * HUD and menus are drawn natively instead of from their GS packets (OPENRAC_UI_REPLAY replays the
 * packets, for comparing). Each routine's quad goes to the window as a record
 * (openrac_game_ui_quad) with the corners and texel coordinates its packet would have held, and the
 * display list gets a marker in its place, so the window draws it where it came.
 * OPENRAC_UI_LOG: each distinct caller and texture of DrawTexturedQuad, once, in the log.
 */
/* Carried from port/game/rac1-pal/host/ui2d.c through the version map (../README.md): names and
 * addresses are this version's. Not yet checked in play on NTSC-U. */
#include "game_protos.h"
#include "openrac/game_host.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The screen offset the game adds to every 2D position (D_0013E500 +0x10/+0x14), and the HUD
 * arena (D_0019A3E8): +0xC the depth its sprites are drawn at, +0x20/+0x24 its texture tables. */
#define SCREEN_OFX GREF(int, 0x0013E510u)
#define SCREEN_OFY GREF(int, 0x0013E514u)
#define HUD_ARENA OPENRAC_DATA(0x0019A3E8u)

static void log_caller(const char* what, unsigned long long tex, int w, int h) {
    static const char* seen[256];
    static int count = 0;
    /* Who called the routine that called this, as the host names it (guest.h: any host). */
    char caller[160];
    openrac_host_caller(1, caller, sizeof(caller));
    char key[256];
    snprintf(key, sizeof(key), "%s%s tbp %#x %dx%d", what, caller, (unsigned)tex & 0x3FFF, w, h);
    for (int i = 0; i < count; ++i) {
        if (strcmp(seen[i], key) == 0) {
            return;
        }
    }
    if (count < 256) {
        seen[count++] = strdup(key);
        fprintf(stderr, "ui2d: %s\n", key);
    }
}

/* The quad to the window and its marker into the display list (DMA cnt and VIF DIRECT of two
 * quadwords, a PACKED GIF tag of one A+D register, the A+D write to NOP carrying the index); false
 * when the 2D layer is replayed, and the caller sends the game's own packet. */
/* OPENRAC_UI_SKIP=NAME,...: those routines send their own packets (bisecting). */
static int skipped(const char* name) {
    const char* list = getenv("OPENRAC_UI_SKIP");
    return list != NULL && strstr(list, name) != NULL;
}

static int emit(openrac_game_ui_rect* q) {
    static int log = -1;
    if (log < 0) {
        log = getenv("OPENRAC_UI_LOG") != NULL;
    }
    if (log) {
        log_caller("emit ", q->tex0, (int)(q->x[3] - q->x[0]) / 16, (int)(q->y[3] - q->y[0]) / 16);
    }
    const int index = openrac_game_ui_quad(q);
    if (index < 0) {
        return 0;
    }
    const gaddr at = GREF(gaddr, OPENRAC_DATA(0x00160F00u));
    GREF(uint32_t, at + 0) = 0x10000002u;
    GREF(uint32_t, at + 4) = 0;
    GREF(uint32_t, at + 8) = 0;
    GREF(uint32_t, at + 12) = 0x50000002u;
    GREF(uint64_t, at + 16) = 1ull | (1ull << 15) | (1ull << 60);
    GREF(uint64_t, at + 24) = 0xEull;
    GREF(uint64_t, at + 32) = (0x4E55ull << 48) | (uint32_t)index;
    GREF(uint64_t, at + 40) = 0x7Full;
    GREF(gaddr, OPENRAC_DATA(0x00160F00u)) = at + 48;
    return 1;
}

/* An axis-aligned quad, (x0, y0) to (x1, y1) on the screen (12.4, offset as the game sends them)
 * and (u0, v0) to (u1, v1) in the texture (12.4), as a strip. */
static void rect(openrac_game_ui_rect* q, int x0, int y0, int x1, int y1, int u0, int v0, int u1, int v1) {
    q->x[0] = (uint32_t)x0, q->y[0] = (uint32_t)y0, q->u[0] = (uint32_t)u0, q->v[0] = (uint32_t)v0;
    q->x[1] = (uint32_t)x1, q->y[1] = (uint32_t)y0, q->u[1] = (uint32_t)u1, q->v[1] = (uint32_t)v0;
    q->x[2] = (uint32_t)x0, q->y[2] = (uint32_t)y1, q->u[2] = (uint32_t)u0, q->v[2] = (uint32_t)v1;
    q->x[3] = (uint32_t)x1, q->y[3] = (uint32_t)y1, q->u[3] = (uint32_t)u1, q->v[3] = (uint32_t)v1;
}

/* A HUD texture's size: 1 << its entry's +6/+7 log2 sizes (the entry found through the arena's
 * +0x20 index table into its +0x24 texture table). */
static void hud_size(int tex, int* tw, int* th) {
    const gaddr arena = HUD_ARENA;
    const gaddr index = GREF(gaddr, arena + 0x20);
    const gaddr e = GREF(gaddr, arena + 0x24) + (gaddr)GREF(int16_t, index + (gaddr)tex * 4 + 2) * 8;
    *tw = 1 << GREF(uint8_t, e + 6);
    *th = 1 << GREF(uint8_t, e + 7);
}

static void hud_common(openrac_game_ui_rect* q, unsigned long long tex0, int alpha) {
    memset(q, 0, sizeof(*q));
    q->flags = 2; /* the HUD's routines send sprites (the turned and rotated ones strips) */
    q->z = (uint32_t)GREF(int, HUD_ARENA + 0xC);
    q->rgba = ((uint32_t)alpha << 24) | 0x7F7F7Fu;
    q->tex0 = tex0;
}

/* Inside FontPrint: the quads drawn are glyphs of text. */
static int g_glyphs = 0;

/* FontPrint: a string at (x, y) in colour, glyph by glyph through DrawTexturedQuad. */
void game_FUN_001f62b0(int x, int y, unsigned long long colour, gaddr text, int limit, long long texture,
                   gaddr glyphs) {
    ++g_glyphs;
    game_FUN_001f62b0__game(x, y, colour, text, limit, texture, glyphs);
    --g_glyphs;
}

/* DrawTexturedQuad: a w x h screen rectangle at (x, y) textured from (u, v) .. (u + uw, v + vh) of
 * the texture tex (TEX0) in colour rgba. */
void game_FUN_001f5450(int x, int y, int w, int h, int u, int v, int uw, int vh, long long rgba_,
                       long long tex_) {
    const unsigned long long rgba = (unsigned long long)rgba_, tex = (unsigned long long)tex_;
    if (skipped("game_FUN_001f5450")) {
        game_FUN_001f5450__game(x, y, w, h, u, v, uw, vh, rgba, tex);
        return;
    }
    static int log = -1;
    if (log < 0) {
        log = getenv("OPENRAC_UI_LOG") != NULL;
    }
    if (log) {
        log_caller("quad ", tex, w, h);
    }
    openrac_game_ui_rect q;
    memset(&q, 0, sizeof(q));
    const int ofx = SCREEN_OFX, ofy = SCREEN_OFY;
    rect(&q, x * 16 + ofx - 8, y * 16 + ofy - 8, (x + w) * 16 + ofx - 8, (y + h) * 16 + ofy - 8, u * 16, v * 16,
         (u + uw) * 16, (v + vh) * 16);
    q.z = 0xFFFFF0u;
    q.rgba = (uint32_t)rgba;
    q.tex0 = tex;
    q.flags = g_glyphs > 0 ? 1u : 0u;
    if (!emit(&q)) {
        game_FUN_001f5450__game(x, y, w, h, u, v, uw, vh, rgba, tex);
    }
}

/* HUD texture tex as a sprite at (x, y), w x h pixels, its whole texture, with alpha. */
void game_FUN_001ffc30(int tex, int x, int y, int w, int h, int alpha) {
    if (skipped("game_FUN_001ffc30")) {
        game_FUN_001ffc30__game(tex, x, y, w, h, alpha);
        return;
    }
    int tw, th;
    openrac_game_ui_rect q;
    hud_size(tex, &tw, &th);
    hud_common(&q, game_FUN_001ffa10(tex), alpha);
    const int ofx = SCREEN_OFX, ofy = SCREEN_OFY;
    rect(&q, x * 16 + ofx - 8, y * 16 + ofy - 8, (x + w) * 16 + ofx - 8, (y + h) * 16 + ofy - 8, 0, 0, tw * 16,
         th * 16);
    if (!emit(&q)) {
        game_FUN_001ffc30__game(tex, x, y, w, h, alpha);
    }
}

/* game_FUN_001ffc30 with the texture turned a quarter: corners (x, y+h), (x, y), (x+w, y+h), (x+w, y)
 * take texels (tw, 0), (tw, th), (0, 0), (0, th). */
void game_FUN_001ffe18(int tex, int x, int y, int w, int h, int alpha) {
    if (skipped("game_FUN_001ffe18")) {
        game_FUN_001ffe18__game(tex, x, y, w, h, alpha);
        return;
    }
    int tw, th;
    openrac_game_ui_rect q;
    hud_size(tex, &tw, &th);
    hud_common(&q, game_FUN_001ffa10(tex), alpha);
    q.flags = 0;
    const int ofx = SCREEN_OFX, ofy = SCREEN_OFY;
    const int xl = x * 16 + ofx - 8, xr = (x + w) * 16 + ofx - 8;
    const int yt = y * 16 + ofy - 8, yb = (y + h) * 16 + ofy - 8;
    const int xs[4] = {xl, xl, xr, xr}, ys[4] = {yb, yt, yb, yt};
    const int us[4] = {tw * 16, tw * 16, 0, 0}, vs[4] = {0, th * 16, 0, th * 16};
    for (int i = 0; i < 4; ++i) {
        q.x[i] = (uint32_t)xs[i], q.y[i] = (uint32_t)ys[i], q.u[i] = (uint32_t)us[i], q.v[i] = (uint32_t)vs[i];
    }
    if (!emit(&q)) {
        game_FUN_001ffe18__game(tex, x, y, w, h, alpha);
    }
}

/* game_FUN_001ffc30 with the position and size in 16ths of a pixel. */
void game_FUN_00200080(int tex, int x, int y, int w, int h, int alpha) {
    if (skipped("game_FUN_00200080")) {
        game_FUN_00200080__game(tex, x, y, w, h, alpha);
        return;
    }
    int tw, th;
    openrac_game_ui_rect q;
    hud_size(tex, &tw, &th);
    hud_common(&q, game_FUN_001ffa10(tex), alpha);
    const int ofx = SCREEN_OFX, ofy = SCREEN_OFY;
    rect(&q, x + ofx - 8, y + ofy - 8, x + w + ofx - 8, y + h + ofy - 8, 0, 0, tw * 16, th * 16);
    if (!emit(&q)) {
        game_FUN_00200080__game(tex, x, y, w, h, alpha);
    }
}

/* game_FUN_00200080 with a texture window: texels from (u, v) on by the texture's size (12.4). */
void game_FUN_00200258(int tex, int x, int y, int w, int h, int u, int v, int alpha) {
    if (skipped("game_FUN_00200258")) {
        game_FUN_00200258__game(tex, x, y, w, h, u, v, alpha);
        return;
    }
    int tw, th;
    openrac_game_ui_rect q;
    hud_size(tex, &tw, &th);
    hud_common(&q, game_FUN_001ffa10(tex), alpha);
    const int ofx = SCREEN_OFX, ofy = SCREEN_OFY;
    rect(&q, x + ofx - 8, y + ofy - 8, x + w + ofx - 8, y + h + ofy - 8, u, v, u + tw * 16, v + th * 16);
    if (!emit(&q)) {
        game_FUN_00200258__game(tex, x, y, w, h, u, v, alpha);
    }
}

/* A sprite of the texture tex0 at (x, y), w x h in 16ths of a pixel, texels from (u, v) on by
 * (1 << (lw + 4), 1 << (lh + 4)), with alpha. */
void game_FUN_00200468(long long tex0, int x, int y, int lw, int lh, int w, int h, int u, int v, int alpha) {
    if (skipped("game_FUN_00200468")) {
        game_FUN_00200468__game(tex0, x, y, lw, lh, w, h, u, v, alpha);
        return;
    }
    openrac_game_ui_rect q;
    hud_common(&q, (unsigned long long)tex0, alpha);
    const int ofx = SCREEN_OFX, ofy = SCREEN_OFY;
    rect(&q, x + ofx - 8, y + ofy - 8, x + w + ofx - 8, y + h + ofy - 8, u, v, u + (1 << (lw + 4)),
         v + (1 << (lh + 4)));
    if (!emit(&q)) {
        game_FUN_00200468__game(tex0, x, y, lw, lh, w, h, u, v, alpha);
    }
}

/* A w x h sprite of tex at (x, y) (16ths of a pixel) turned by rot: corners pos +- a +- b with
 * a = h (sin, cos), b = w (-cos, sin), texels from (0, 0) to (u, v) texels. */
void game_FUN_00200600(int u, int v, long long tex, float x, float y, float w, float h, float rot) {
    if (skipped("game_FUN_00200600")) {
        game_FUN_00200600__game(u, v, tex, x, y, w, h, rot);
        return;
    }
    openrac_game_ui_rect q;
    hud_common(&q, (unsigned long long)tex, 0x80);
    q.flags = 0;
    q.rgba = 0x807F7F7Fu;
    const float s = game_FUN_001f9de0(rot), c = game_FUN_001f9dc8(rot);
    const float ax = h * s, ay = h * c, bx = -w * c, by = w * s;
    const float cx[4] = {x + ax - bx, x + ax + bx, x - ax - bx, x - ax + bx};
    const float cy[4] = {y + ay - by, y + ay + by, y - ay - by, y - ay + by};
    const int us[4] = {u * 16, u * 16, 0, 0}, vs[4] = {0, v * 16, 0, v * 16};
    const int ofx = SCREEN_OFX, ofy = SCREEN_OFY;
    for (int i = 0; i < 4; ++i) {
        q.x[i] = (uint32_t)((int)cx[i] + ofx - 8);
        q.y[i] = (uint32_t)((int)cy[i] + ofy - 8);
        q.u[i] = (uint32_t)us[i];
        q.v[i] = (uint32_t)vs[i];
    }
    if (!emit(&q)) {
        game_FUN_00200600__game(u, v, tex, x, y, w, h, rot);
    }
}

/* HUD texture tex as a sprite from (x0, y0) to (x1, y1) (16ths of a pixel), texels (u0, v0) to
 * (u1, v1) (12.4), with alpha. */
void game_FUN_00200958(int tex, int x0, int y0, int x1, int y1, int u0, int v0, int u1, int v1, int alpha) {
    if (skipped("game_FUN_00200958")) {
        game_FUN_00200958__game(tex, x0, y0, x1, y1, u0, v0, u1, v1, alpha);
        return;
    }
    openrac_game_ui_rect q;
    hud_common(&q, game_FUN_001ffa10(tex), alpha);
    const int ofx = SCREEN_OFX, ofy = SCREEN_OFY;
    rect(&q, x0 + ofx - 8, y0 + ofy - 8, x1 + ofx - 8, y1 + ofy - 8, u0, v0, u1, v1);
    if (!emit(&q)) {
        game_FUN_00200958__game(tex, x0, y0, x1, y1, u0, v0, u1, v1, alpha);
    }
}
