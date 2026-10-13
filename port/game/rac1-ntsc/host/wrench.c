/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (NTSC-U): the wrench swing's trail, a draw callback that also keeps state
 * (hostgen.json, "host_functions"). Written from the retail routine (and the decompilation's near
 * miss nonmatching/shared/game_FUN_L00_002a7780.c): the state it keeps is here; the quads it builds go to
 * the console's quad renderer (func_L00_001FD228), which the port does not run, so they are left out.
 * Host C rather than a candidate because the source file declares the callback as void (void).
 */
/* Carried from port/game/rac1-pal/host/wrench.c through the version map (../README.md): names and
 * addresses are this version's. Not yet checked in play on NTSC-U. */
#include "game_protos.h"
#include "openrac/game_host.h"

#include <stdint.h>
#include <string.h>

/* The hero block's moby (D_0013F350 + 0x2080). */
#define HERO_MOBY 0x001413D0u

static inline float f32(gaddr a) {
    float f;
    memcpy(&f, G(a), 4);
    return f;
}
static inline void setf(gaddr a, float f) { memcpy(G(a), &f, 4); }

/* game_FUN_L00_002a7780(moby): the trail record o = moby +0x78; its phase +0x70 (0: off). On the first
 * tick (phase 1.0) it stops when Ratchet's animation is changing or none, else records his rows
 * (o +0, +0x10, +0x20) and restarts the frame stamp. The swing kind +0x78 (0x17..0x19, else 0x2B)
 * picks one of four 0x60-byte rows of D_L00_001D92E8; the phase advances by the row's rate times the
 * frames passed (D_L00_0015F5CC - D_L00_001613C0, at least 1) and ends (0 unless the swing goes on)
 * past twice the row's segment count; while it runs, o +0x30 = Ratchet's position. */
void game_FUN_L00_002a7780(gaddr s) {
    const gaddr frame = OPENRAC_LDATA(0, 0x0015F5CCu);
    const gaddr stamp = OPENRAC_LDATA(0, 0x001613C0u);
    const gaddr table = OPENRAC_LDATA(0, 0x001D92E8u);
    const gaddr o = GREF(gaddr, s + 0x78);
    if (o == 0) {
        return;
    }
    float f2 = f32(o + 0x70);
    if (f2 == 0.0f) {
        return;
    }
    gaddr pl = GREF(gaddr, HERO_MOBY);
    if (f2 == 1.0f) {
        if (GREF(uint8_t, pl + 0x52) != GREF(uint8_t, pl + 0x53) || GREF(uint8_t, pl + 0x52) == 0xFF) {
            setf(o + 0x70, 0.0f);
            return;
        }
    }
    const int kind = GREF(uint8_t, pl + 0x52);
    int v = ((unsigned)(kind - 0x17) < 3 || kind == 0x2B) ? kind : GREF(int16_t, o + 0x78);
    int sel;
    if (v == 0x17 || v == 0x18 || v == 0x19) {
        sel = v - 0x17;
    } else {
        v = 0x2B;
        sel = 3;
    }
    GREF(int16_t, o + 0x78) = (int16_t)v;
    const gaddr tbl = table + (gaddr)sel * 0x60;
    f2 = f32(o + 0x70);
    if (f2 == 1.0f) {
        memmove(G(o), G(pl + 0xC0), 16);
        memmove(G(o + 0x10), G(pl + 0xD0), 16);
        memmove(G(o + 0x20), G(pl + 0xE0), 16);
        GREF(int, stamp) = GREF(int, frame);
    }
    const int d = GREF(int, frame) - GREF(int, stamp);
    const int n = d > 0 ? d : 1;
    const float f1 = f2 + (float)n * f32(tbl + 0x48);
    setf(o + 0x70, f1);
    GREF(int, stamp) = GREF(int, frame);
    const int h = GREF(int16_t, tbl + 0x58);
    if ((float)(h * 2) <= f1) {
        const int k = GREF(uint8_t, pl + 0x52);
        if ((unsigned)(k - 0x17) < 3) {
            return;
        }
        if (k != 0x2B) {
            setf(o + 0x70, 0.0f);
        }
        return;
    }
    memmove(G(o + 0x30), G(pl + 0x10), 16);
}
