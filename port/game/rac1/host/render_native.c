/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: the game's own renderers and the loaders that prepare their data, which the
 * native port does not run. In the retail program they are hand-written assembly (or not decompiled
 * yet) that turns level data into VU1 and GS packets; the port draws the same things with its own
 * renderers (port/renderer, game/common/frontend.cpp) from the extracted level, so here they do
 * nothing. Each is where a native renderer takes over.
 */
#include "rac1.h"
#include "moby_view.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

/* a quad renderer */
void func_001F8B6C(void) {
}

/* a quad renderer */
void func_001F91B8(void) {
}

/*
 * shrub_class_init: registers a shrub class (a0) under its class number (a4) and slot. What the level
 * loader reads afterwards is kept: the slot tables (class number to slot, slot to class number and to
 * class), the class's draw distance, its offsets made pointers, and its 16-byte block (a2). What is
 * left out is the GS texture registers it writes into the class's packets (a1, a3), which only the
 * game's shrub renderer reads. In a level, its tables are the level program's copies.
 */
void func_00204340(gaddr a0, gaddr a1, gaddr a2, gaddr a3, int a4) {
    (void)a1;
    (void)a3;
    uint8_t* cls = G(a0);
    const uint32_t slot = GREF(uint32_t, OPENRAC_DATA(RAC1_ADDR(0x001604CCu)));

    GREF(uint8_t, OPENRAC_DATA(RAC1_ADDR(0x001D8440u)) + (uint32_t)a4) = (uint8_t)slot;
    GREF(uint16_t, OPENRAC_DATA(RAC1_ADDR(0x001D83C0u)) + slot * 2) = (uint16_t)a4;
    GREF(gaddr, OPENRAC_DATA(RAC1_ADDR(0x001D82C0u)) + slot * 4) = a0;
    *(uint16_t*)(cls + 0x26) = (uint16_t)slot;

    float distance;
    memcpy(&distance, cls + 0x10, 4);
    GREF(int, OPENRAC_DATA(RAC1_ADDR(0x001D9040u)) + slot * 4) = func_001FA898(distance * 1024.0f);
    GREF(uint32_t, OPENRAC_DATA(RAC1_ADDR(0x001604CCu))) = slot + 1;

    *(uint32_t*)(cls + 0x18) = 0;
    *(uint16_t*)(cls + 0x16) = 0;
    if (*(uint32_t*)(cls + 0x2C) != 0) {
        *(uint32_t*)(cls + 0x2C) += a0;
    }
    const int16_t packets = *(int16_t*)(cls + 0x28);
    for (int i = 0; i < packets; ++i) {
        *(uint32_t*)(cls + 0x40 + i * 8) += a0;
    }
    if (*(uint32_t*)(cls + 0x1C) != 0) {
        *(uint32_t*)(cls + 0x1C) += a0;
    }

    memcpy(G(OPENRAC_DATA(RAC1_ADDR(0x001D9640u)) + slot * 16), G(a2), 16);
}

/*
 * A marker in the display list, where the game's write pointer (D_00161000) is: a DMA cnt of two
 * quadwords with VIF DIRECT, a PACKED GIF tag of one A+D register, then the A+D write of `value` to
 * the NOP register, as ui2d.c's quad markers. The chip ignores it; the window reads its top 16 bits
 * (renderer::GifInterpreter's kUiQuadMarker, kMobyMarker and kWorldMarker).
 */
static void put_marker(uint64_t value) {
    const gaddr at = GREF(gaddr, OPENRAC_DATA(RAC1_ADDR(0x00161000u)));
    GREF(uint32_t, at + 0) = 0x10000002u;
    GREF(uint32_t, at + 4) = 0;
    GREF(uint32_t, at + 8) = 0;
    GREF(uint32_t, at + 12) = 0x50000002u;
    GREF(uint64_t, at + 16) = 1ull | (1ull << 15) | (1ull << 60);
    GREF(uint64_t, at + 24) = 0xEull;
    GREF(uint64_t, at + 32) = value;
    GREF(uint64_t, at + 40) = 0x7Full;
    GREF(gaddr, OPENRAC_DATA(RAC1_ADDR(0x00161000u))) = at + 48;
}

/* a moby texture DMA */
void func_00212258(int a0) {
    (void)a0;
}

/* a moby renderer step */
void func_00212508(void) {
}

/* MobyAnimProc (the draw side) */
void func_00212578(int a0, int a1) {
    (void)a0;
    (void)a1;
}

/*
 * What MobyProc decides for each moby of the list before drawing it, which the game reads back:
 * +0x31, drawn this frame (the moby loop keeps a drawn moby active; vehicles, enemies and effects
 * test it). The list runs COUNT mobys of 0x100 bytes from FIRST (a negative count: on to its end),
 * and ends early at a moby whose state (+0x20) is -1, the list's end marker; a moby with another
 * negative state (being removed) is passed over, its +0x31 left as it was. Not drawn: a moby whose
 * group mask (+0x36) has no bit of the current one (the table at D_001940C0, which MobyProc copies
 * to the scratchpad, at +0x37), a hidden one (mode +0x34 bits 0x81), and, as ReRAC's moby_lod.rs
 * replays the culls, a bounding sphere (+0x00, x y z r in units of 1/1024) beyond the draw distance
 * (+0x32, s16 units, at most the global one at D_00160030), wholly in front of the near plane, or
 * wholly outside a side plane of the view. The side planes are the port's picture's, wider than
 * 4:3 when it is (moby_view.h).
 */
static void mark_drawn_mobys(gaddr first, int count) {
    float cam[3], rows[12], view[12];
    memcpy(cam, G(OPENRAC_DATA(RAC1_ADDR(0x00187180u))), sizeof(cam));
    /* The camera's rotation as MobyProc loads it (lqc2 of gCamera's first three rows, 0x187040):
     * BuildViewMatrices makes them from the forward, side and up rows (z, x, y) of 0x187390, or
     * from the camera's angles, column by column: forward is column 2, side and up columns 0 and 1
     * negated. Read back so, they are those rows bit for bit, whichever way the frame made them. */
    memcpy(view, G(OPENRAC_DATA(RAC1_ADDR(0x00187040u))), sizeof(view));
    for (int c = 0; c < 3; ++c) {
        rows[c] = view[c * 4 + 2];
        rows[4 + c] = -view[c * 4 + 0];
        rows[8 + c] = -view[c * 4 + 1];
    }
    rows[3] = rows[7] = rows[11] = 0.0f;
    float tx, ty;
    memcpy(&tx, G(OPENRAC_DATA(RAC1_ADDR(0x0018CE00u)) + 0xB0), 4);
    memcpy(&ty, G(OPENRAC_DATA(RAC1_ADDR(0x0018CE00u)) + 0xB4), 4);
    if (!(tx > 0.05f && tx < 10.0f)) {
        tx = 0.63f;
    }
    if (!(ty > 0.05f && ty < 10.0f)) {
        ty = tx * 0.756f;
    }
    /* The picture's sides: the game's 4:3 tangent opened as the port's picture is (moby_view.h).
     * The game fix fix_wide_moby_effects (differences.json); off, the console's 4:3 test. */
    static const unsigned char* fix;
    if (!fix) {
        fix = openrac_game_fix("fix_wide_moby_effects");
    }
    const float wider = *fix ? openrac_game_picture_wider() : 1.0f;
    const gaddr end = first + (gaddr)count * 0x100u;
    const int max_distance = GREF(int, OPENRAC_DATA(RAC1_ADDR(0x00160030u)));
    const uint8_t* groups = G(OPENRAC_DATA(RAC1_ADDR(0x001940C0u)));
    for (gaddr m = first; count < 0 || m != end; m += 0x100) {
        if (m - first >= 0x100u * 4096u) {
            break;
        }
        uint8_t* b = G(m);
        const int8_t state = (int8_t)b[0x20];
        if (state == -1) {
            break;  // the end marker
        }
        if (state < 0) {
            continue;
        }
        const uint16_t mode = *(uint16_t*)(b + 0x34);
        int drawn = (b[0x36] & groups[b[0x37]]) != 0 && (mode & 0x81) == 0;
        if (drawn) {
            float sphere[4];
            memcpy(sphere, b, sizeof(sphere));
            const float r = sphere[3] / 1024.0f;
            const float d[3] =
                {sphere[0] / 1024.0f - cam[0],
                 sphere[1] / 1024.0f - cam[1],
                 sphere[2] / 1024.0f - cam[2]};
            int distance = *(int16_t*)(b + 0x32);
            if (distance > max_distance) {
                distance = max_distance;  // pminw with the global draw distance
            }
            drawn = moby_in_view(d, rows, r, (float)distance, tx, ty, wider);
        }
        b[0x31] = (uint8_t)(drawn ? 1 : 0);
    }
}

/* the moby renderer */
int func_00212658(int a0, int a1, int a2, int a3) {
    mark_drawn_mobys((gaddr)a0, a2);
    openrac_game_draw(OPENRAC_DRAW_MOBYS);
    if (openrac_game_mobys_drawn((gaddr)a0, a2)) {
        /* A moby marker in the display list (put_marker), the count in bits 32-39 and the first
         * moby in the low word: the mobys of a screen the game renders to a texture (the vendor's
         * item and salesman) are drawn into it there. */
        put_marker((0x4D42ull << 48) | ((uint64_t)(a2 & 0xFF) << 32) | (uint32_t)a0);
    }
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    return 0;
}

/*
 * PartProc, the particle renderer: each live record of the pool (2048 records of 0x40 bytes at the
 * pool pointer; byte 1 bit 7 marks a free one; the highest live index is kept with the pool's
 * counters) goes to the window, which draws it as the game's VU1 sprite program does
 * (openrac_game_particle). Its texture is entry +0x02 of the particle texture table, 128 words
 * 0x200 bytes past the pool's allocation bitmap (level 0's 0x1B1E00, which PartProc copies to the
 * scratchpad each frame) read at entry * 8: the low half the palette's address << 4 | the CLUT
 * offset, the high half the pixels' address << 4. The loader (func_00219704) always gives the
 * texture 32 x 32 (0x505). Read as two words an entry with the size from a nibble, most particles
 * named another texture or a single texel: the explosions' smoke came out as opaque white or noisy
 * squares, and the nanotech crates' orbs as nothing.
 */
void func_00218B10(void) {
    if (openrac_guest_overlay() < 0) {
        return;  // the boot program (title, menus, the flight) keeps no particle pool there
    }
    // The pool's globals are the level programs' own (level 0's addresses, relocated to the level
    // loaded); the boot program keeps other things there.
    const gaddr pool = GREF(gaddr, OPENRAC_LDATA(0, RAC1_LADDR(00, 0x0016022Cu)));
    const int high = GREF(int, OPENRAC_LDATA(0, RAC1_LADDR(00, 0x00160234u)));
    const gaddr table = OPENRAC_LDATA(0, RAC1_LADDR(00, 0x001B1C00u)) + 0x200;
    if (pool == 0 || pool >= 0x01FE0000u) {
        return;
    }
    for (int i = 0; i <= high && i < 2048; ++i) {
        const uint8_t* r = G(pool + (gaddr)i * 0x40);
        if ((r[1] & 0x80) != 0) {
            continue;
        }
        uint64_t entry;
        memcpy(&entry, G(table + (gaddr)r[2] * 8), 8);
        openrac_game_particle(r, (uint32_t)(entry >> 32) >> 4, (uint32_t)entry >> 4, 5);
    }
}

/* a shadow renderer */
void func_00228A58(void) {
}

/* a shadow renderer */
void func_00228D20(unsigned int a0, int a1, int a2) {
    (void)a0;
    (void)a1;
    (void)a2;
}

/* a shadow renderer */
int func_00229098(void) {
    return 0;
}

/* a shadow renderer */
int func_002291E8(void) {
    return 0;
}

/* a shadow renderer */
void func_00229838(gaddr a0, gaddr a1) {
    (void)a0;
    (void)a1;
}

/* a shadow renderer */
void func_002298B0(int a0, int a1) {
    (void)a0;
    (void)a1;
}

/* ShrubProc, the shrub renderer */
void func_00229F00(void) {
    openrac_game_draw(OPENRAC_DRAW_SHRUBS);
}

/* BuildShrubTextureDma */
int func_0022B648(int a0) {
    (void)a0;
    return 0;
}

/* LightShrubs(list): lights the instances of the 0xFFFF-ended index list at `list` (the scratchpad
 * at level load, the frame's buffer after). One argument: the retail code writes $a1 before it reads
 * it, so callers pass only the list. */
void func_0022B8F8(gaddr a0) {
    (void)a0;
}

/*
 * SkyDrawShell(shell): the sky code (the levels' skies, the flight's DrawSkyShells) builds each
 * shell's matrix at D_001D9A70 before drawing it: its turn, scale and, in the flight, the
 * translation that moves the stars past the ship. The window draws the shells natively with it.
 */
void func_0022C9A8(int shell) {
    float m[16];
    memcpy(m, G(OPENRAC_DATA(RAC1_ADDR(0x001D9A70u))), sizeof(m));
    openrac_game_sky_shell(shell, m);
    func_0022C9A8__game(shell);
}

/* SkyDrawShellTextured */
void func_0022CA00(gaddr a0) {
    openrac_game_draw(OPENRAC_DRAW_SKY);
    (void)a0;
}

/* SkyDrawShellGouraud */
void func_0022CC40(gaddr a0) {
    openrac_game_draw(OPENRAC_DRAW_SKY);
    (void)a0;
}

/*
 * SkySpriteProc: the sky's sprites (stars, glows), from the sky header (+0x08 their count, +0x10 the
 * texture table, 16 bytes an entry with the TEX0 first, +0x1C the 0x20-byte records; a negative
 * texture byte +0x02 is not drawn), each to the window (openrac_game_sky_sprite).
 */
void func_0022CEB8(void) {
    openrac_game_draw(OPENRAC_DRAW_SKY);
    const gaddr sky = GREF(gaddr, OPENRAC_DATA(RAC1_ADDR(0x0016055Cu)));
    if (sky == 0 || sky >= 0x01FFF000u) {
        return;
    }
    const gaddr records = GREF(gaddr, sky + 0x1C);
    const int count = GREF(uint16_t, sky + 0x08);
    const gaddr textures = GREF(gaddr, sky + 0x10);
    if (records == 0 || textures == 0 || records >= 0x01FFF000u) {
        return;
    }
    for (int i = 0; i < count && i < 1024; ++i) {
        const uint8_t* r = G(records + (gaddr)i * 0x20);
        if ((int8_t)r[2] < 0) {
            continue;
        }
        uint64_t tex0;
        memcpy(&tex0, G(textures + (gaddr)r[2] * 16), 8);
        openrac_game_sky_sprite(r, tex0);
    }
}

/*
 * TfragProc, the terrain renderer. The world's marker goes into the display list here (kWorldMarker,
 * 0x574C in the top 16 bits): DrawWorld's first native pass after the sky, where the window draws
 * the world. The 2D path before it (the frame's clear and blur, the sky's setup) is drawn before the
 * world, the rest (the draw callbacks' effects, the HUD, the fades) after it. The first VU1 program
 * a frame started stood for it before, but the port's world renderers send none, and the first is
 * the world text's sprite pass (func_001F7B70), which runs only while there is any: on Kerwan in
 * play there was none and the HUD went under the world; on Rilgar the split came and went.
 */
void func_002352C8(void) {
    openrac_game_draw(OPENRAC_DRAW_TERRAIN);
    put_marker(0x574Cull << 48);
}

/* ComputeTfragTextureUsage */
void func_00235EF0(void) {
}

/* the tfrag texture DMA */
int func_00236060(int a0) {
    (void)a0;
    return 0;
}

/* LightTfrags */
void func_002362B0(gaddr a0) {
    (void)a0;
}

/* TieProc, the tie renderer */
void func_00236F00(void) {
    openrac_game_draw(OPENRAC_DRAW_TIES);
}

/* BuildTieTextureDma */
int func_002383D8(int a0) {
    (void)a0;
    return 0;
}

/* LightTies(list): lights the instances of the 0xFFFF-ended index list at `list` (the scratchpad
 * at level load, the frame's buffer after). One argument: the retail code writes $a1 before it reads
 * it, so callers pass only the list. */
void func_00238688(gaddr a0) {
    (void)a0;
}
/* PatchMobyGifs: texture addresses into the GS packets of the console's renderer */
void func_0020DD48(void) {
}

/* PatchShrubGifs: texture addresses into the GS packets of the console's renderer */
void func_00229D48(void) {
}

/* PatchTfragGifs: texture addresses into the GS packets of the console's renderer */
void func_00234620(void) {
}

/* PatchTieGifs: texture addresses into the GS packets of the console's renderer */
void func_00236A98(void) {
}
/* drawquad.c: a renderer of the console's (the port draws natively) */
int func_001F7C60(void) {
    return 0;
}

/* drawquad.c: a renderer of the console's (the port draws natively) */
int func_001F7DD8(void) {
    return 0;
}

/* drawquad.c: a renderer of the console's (the port draws natively) */
int func_001F7E98(void) {
    return 0;
}

/* drawquad.c: a renderer of the console's (the port draws natively) */
int func_001F84AC(void) {
    return 0;
}

/* drawquad.c: a renderer of the console's (the port draws natively) */
int func_001F852C(void) {
    return 0;
}

/* drawquad.c: a renderer of the console's (the port draws natively) */
int func_001F856C(void) {
    return 0;
}

/* skyproc.c: a renderer of the console's (the port draws natively) */
int func_0022D2AC(int a0, int a1, int a2, int a3) {
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    return 0;
}

/* skyproc.c: a renderer of the console's (the port draws natively) */
void func_0022D3F8(int a0, int a1, int a2, int a3) {
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
}

/* skyproc.c: a renderer of the console's (the port draws natively) */
void func_0022D520(int a0, int a1, int a2, int a3) {
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
}

/* skyproc.c: a renderer of the console's (the port draws natively) */
void func_0022D7E0(gaddr a0, int a1, gaddr a2) {
    (void)a0;
    (void)a1;
    (void)a2;
}

/* drawquad_001FD1D8.c: a renderer of the console's (the port draws natively) */
int func_L00_001FE688(void) {
    return 0;
}

/* skyproc_0028D958.c: a renderer of the console's (the port draws natively) */
int func_L00_0028D958(void) {
    return 0;
}

/* tfragproc_002963D8.c: a renderer of the console's (the port draws natively) */
int func_L00_002963D8(void) {
    return 0;
}

/* moby_anim_eval_chain (hand-written VU0, 2640 bytes): the pose matrices of the joints marked in
 * a1 for the moby a0, into the scratchpad (MobyGetBoneMatrix reads them: the menu panels' corners,
 * items in Ratchet's hands). The port's evaluator (viewer/moby_pose, after ReRAC's evaluate_chains;
 * post-scale records are not skipped yet as the chain form does). */
void func_00211808(gaddr a0, gaddr a1) {
    openrac_game_moby_chain(a0, a1);
}

#if !RAC1_OMIT(func_L00_00254218)
/* The level programs' joint pose (hand-written VU0 with scratchpad DMA, 2136 bytes): the moby a0's
 * joints 0 to a1 - 1 posed into the scratchpad, 0x40 bytes a joint, where the collision primitive
 * code (mobyfunc_002649C8.c) reads a joint's row. The port's evaluator, as func_00211808. Not made:
 * retail's copy of the matrices into the buffer at the moby's +0x24, and its use of a2 (0 at every
 * call in the C). Hitting a crate on Kerwan called it. */
void func_L00_00254218(gaddr a0, short a1, int a2) {
    static gaddr marks;
    int count = a1 > 0x7F ? 0x7F : a1;
    (void)a2;
    if (count <= 0) {
        return;
    }
    if (marks == 0) {
        marks = openrac_guest_static(0x80);
    }
    memset(G(marks), 0, 0x80);
    memset(G(marks), 1, (size_t)count);
    *(uint8_t*)G(marks + 0x7F) = (uint8_t)count;
    openrac_game_moby_chain(a0, marks);
}
#endif
