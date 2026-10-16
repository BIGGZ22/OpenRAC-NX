/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (NTSC-U): the game's own renderers and the loaders that prepare their data, which the
 * native port does not run. In the retail program they are hand-written assembly (or not decompiled
 * yet) that turns level data into VU1 and GS packets; the port draws the same things with its own
 * renderers (port/renderer, game/common/frontend.cpp) from the extracted level, so here they do
 * nothing. Each is where a native renderer takes over.
 */
/* Carried from port/game/rac1-pal/host/render_native.c through the version map (../README.md): names and
 * addresses are this version's. Not yet checked in play on NTSC-U. */
#include "game_protos.h"
#include "openrac/game_host.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

/* a quad renderer */
void game_FUN_001f89a4(void) {
}

/* a quad renderer */
void game_FUN_001f8ff0(void) {
}

/*
 * shrub_class_init: registers a shrub class (a0) under its class number (a4) and slot. What the level
 * loader reads afterwards is kept: the slot tables (class number to slot, slot to class number and to
 * class), the class's draw distance, its offsets made pointers, and its 16-byte block (a2). What is
 * left out is the GS texture registers it writes into the class's packets (a1, a3), which only the
 * game's shrub renderer reads. In a level, its tables are the level program's copies.
 */
void game_FUN_00203b08(gaddr a0, gaddr a1, gaddr a2, gaddr a3, int a4) {
    (void)a1;
    (void)a3;
    uint8_t* cls = G(a0);
    const uint32_t slot = GREF(uint32_t, OPENRAC_DATA(0x001603CCu));

    GREF(uint8_t, OPENRAC_DATA(0x001D80B0u) + (uint32_t)a4) = (uint8_t)slot;
    GREF(uint16_t, OPENRAC_DATA(0x001D8030u) + slot * 2) = (uint16_t)a4;
    GREF(gaddr, OPENRAC_DATA(0x001D7F30u) + slot * 4) = a0;
    *(uint16_t*)(cls + 0x26) = (uint16_t)slot;

    float distance;
    memcpy(&distance, cls + 0x10, 4);
    GREF(int, OPENRAC_DATA(0x001D8CB0u) + slot * 4) = game_FUN_001fa6d0(distance * 1024.0f);
    GREF(uint32_t, OPENRAC_DATA(0x001603CCu)) = slot + 1;

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

    memcpy(G(OPENRAC_DATA(0x001D92B0u) + slot * 16), G(a2), 16);
}

/* a moby texture DMA */
void game_FUN_00211408(int a0) {
    (void)a0;
}

/* a moby renderer step */
void game_FUN_002116b8(void) {
}

/* MobyAnimProc (the draw side) */
void game_FUN_00211728(int a0, int a1) {
    (void)a0;
    (void)a1;
}

/*
 * What MobyProc decides for each moby of the list before drawing it, which the game reads back:
 * +0x31, drawn this frame (the moby loop keeps a drawn moby active; vehicles, enemies and effects
 * test it). Its culls, as ReRAC's moby_lod.rs replays them: the bounding sphere (+0x00, x y z r in
 * units of 1/1024) beyond the draw distance (+0x32, s16 units), wholly in front of the near plane,
 * or wholly outside a side plane of the view; a hidden moby (mode bit 0) is not drawn.
 */
static void mark_drawn_mobys(gaddr first, int count) {
    float cam[3], rows[12];
    memcpy(cam, G(OPENRAC_DATA(0x00187080u)), sizeof(cam));
    memcpy(rows, G(OPENRAC_DATA(0x00187290u)), sizeof(rows));
    float tx, ty;
    memcpy(&tx, G(OPENRAC_DATA(0x0018CD00u) + 0xB0), 4);
    memcpy(&ty, G(OPENRAC_DATA(0x0018CD00u) + 0xB4), 4);
    if (!(tx > 0.05f && tx < 10.0f)) {
        tx = 0.63f;
    }
    if (!(ty > 0.05f && ty < 10.0f)) {
        ty = tx * 0.756f;
    }
    const float kx = sqrtf(1.0f + tx * tx), ky = sqrtf(1.0f + ty * ty);
    gaddr end = count < 0 ? GREF(gaddr, OPENRAC_DATA(0x0015FF20u)) : first + (gaddr)count * 0x100u;
    if (end < first || end - first > 0x100u * 4096u) {
        return;
    }
    for (gaddr m = first; m < end; m += 0x100) {
        uint8_t* b = G(m);
        float sphere[4];
        memcpy(sphere, b, sizeof(sphere));
        const float r = sphere[3] / 1024.0f;
        const float d[3] = {sphere[0] / 1024.0f - cam[0], sphere[1] / 1024.0f - cam[1], sphere[2] / 1024.0f - cam[2]};
        const float z = d[0] * rows[0] + d[1] * rows[1] + d[2] * rows[2];
        const float x = d[0] * rows[4] + d[1] * rows[5] + d[2] * rows[6];
        const float y = d[0] * rows[8] + d[1] * rows[9] + d[2] * rows[10];
        const int distance = *(int16_t*)(b + 0x32);
        const uint16_t mode = *(uint16_t*)(b + 0x34);
        int drawn = (mode & 1) == 0 && b[0x20] != 0xFF;
        drawn = drawn && z <= (float)distance && z + r > 32.0f / 1024.0f;
        drawn = drawn && fabsf(x) - r * kx <= tx * z && fabsf(y) - r * ky <= ty * z;
        b[0x31] = (uint8_t)(drawn ? 1 : 0);
    }
}

/* the moby renderer */
int game_FUN_00211808(int a0, int a1, int a2, int a3) {
    mark_drawn_mobys((gaddr)a0, a2);
    openrac_game_draw(OPENRAC_DRAW_MOBYS);
    openrac_game_mobys_drawn((gaddr)a0, a2);
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
 * (openrac_game_particle). Its texture is entry +0x02 of the particle texture table, which sits
 * 0x200 bytes past the pool's allocation bitmap: (palette address << 4 | CLUT offset, pixels address
 * << 4 | log2 side).
 */
void game_FUN_00217c18(void) {
    if (openrac_guest_overlay() < 0) {
        return;  // the boot program (title, menus, the flight) keeps no particle pool there
    }
    // The pool's globals are the level programs' own (level 0's addresses, relocated to the level
    // loaded); the boot program keeps other things there.
    const gaddr pool = GREF(gaddr, OPENRAC_LDATA(0, 0x0016016Cu));
    const int high = GREF(int, OPENRAC_LDATA(0, 0x00160174u));
    const gaddr table = OPENRAC_LDATA(0, 0x001B1880u) + 0x200;
    if (pool == 0 || pool >= 0x01FE0000u || table >= 0x01FFF000u) {
        return;
    }
    for (int i = 0; i <= high && i < 2048; ++i) {
        const uint8_t* r = G(pool + (gaddr)i * 0x40);
        if ((r[1] & 0x80) != 0) {
            continue;
        }
        uint64_t lo, hi;
        memcpy(&lo, G(table + (gaddr)r[2] * 16), 8);
        memcpy(&hi, G(table + (gaddr)r[2] * 16 + 8), 8);
        openrac_game_particle(r, (uint32_t)(hi >> 4), (uint32_t)(lo >> 4), (int)(hi & 0xF));
    }
}

/* a shadow renderer */
void game_FUN_00227740(void) {
}

/* a shadow renderer */
void game_FUN_00227a08(unsigned int a0, int a1, int a2) {
    (void)a0;
    (void)a1;
    (void)a2;
}

/* a shadow renderer */
int game_FUN_00227d80(void) {
    return 0;
}

/* a shadow renderer */
int game_FUN_00227ed0(void) {
    return 0;
}

/* a shadow renderer */
void game_FUN_00228520(gaddr a0, gaddr a1) {
    (void)a0;
    (void)a1;
}

/* a shadow renderer */
void game_FUN_00228598(int a0, int a1) {
    (void)a0;
    (void)a1;
}

/* ShrubProc, the shrub renderer */
void game_FUN_00228be8(void) {
    openrac_game_draw(OPENRAC_DRAW_SHRUBS);
}

/* BuildShrubTextureDma */
int game_FUN_0022a330(int a0) {
    (void)a0;
    return 0;
}

/* LightShrubs */
void game_FUN_0022a5e0(gaddr a0, gaddr a1) {
    (void)a0;
    (void)a1;
}

/* SkyDrawShellTextured */
void game_FUN_0022b6e8(gaddr a0) {
    openrac_game_draw(OPENRAC_DRAW_SKY);
    (void)a0;
}

/* SkyDrawShellGouraud */
void game_FUN_0022b928(gaddr a0) {
    openrac_game_draw(OPENRAC_DRAW_SKY);
    (void)a0;
}

/*
 * SkySpriteProc: the sky's sprites (stars, glows), from the sky header (+0x08 their count, +0x10 the
 * texture table, 16 bytes an entry with the TEX0 first, +0x1C the 0x20-byte records; a negative
 * texture byte +0x02 is not drawn), each to the window (openrac_game_sky_sprite).
 */
void game_FUN_0022bba0(void) {
    openrac_game_draw(OPENRAC_DRAW_SKY);
    const gaddr sky = GREF(gaddr, OPENRAC_DATA(0x0016045Cu));
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

/* TfragProc, the terrain renderer */
void game_FUN_00233fb0(void) {
    openrac_game_draw(OPENRAC_DRAW_TERRAIN);
}

/* ComputeTfragTextureUsage */
void game_FUN_00234bd8(void) {
}

/* the tfrag texture DMA */
int game_FUN_00234d48(int a0) {
    (void)a0;
    return 0;
}

/* LightTfrags */
void game_FUN_00234f98(gaddr a0) {
    (void)a0;
}

/* TieProc, the tie renderer */
void game_FUN_00235be8(void) {
    openrac_game_draw(OPENRAC_DRAW_TIES);
}

/* BuildTieTextureDma */
int game_FUN_002370c0(int a0) {
    (void)a0;
    return 0;
}

/* LightTies */
void game_FUN_00237370(gaddr a0, gaddr a1) {
    (void)a0;
    (void)a1;
}
/* PatchMobyGifs: texture addresses into the GS packets of the console's renderer */
void game_FUN_0020cef8(void) {
}

/* PatchShrubGifs: texture addresses into the GS packets of the console's renderer */
void game_FUN_00228a30(void) {
}

/* PatchTfragGifs: texture addresses into the GS packets of the console's renderer */
void game_FUN_00233308(void) {
}

/* PatchTieGifs: texture addresses into the GS packets of the console's renderer */
void game_FUN_00235780(void) {
}
/* drawquad.c: a renderer of the console's (the port draws natively) */
int game_FUN_001f7a98(void) {
    return 0;
}

/* drawquad.c: a renderer of the console's (the port draws natively) */
int func_001F7C10(void) {
    return 0;
}

/* drawquad.c: a renderer of the console's (the port draws natively) */
int func_001F7CD0(void) {
    return 0;
}

/* drawquad.c: a renderer of the console's (the port draws natively) */
int game_FUN_001f82e4(void) {
    return 0;
}

/* drawquad.c: a renderer of the console's (the port draws natively) */
int game_FUN_001f8364(void) {
    return 0;
}

/* drawquad.c: a renderer of the console's (the port draws natively) */
int game_FUN_001f83a4(void) {
    return 0;
}

/* skyproc.c: a renderer of the console's (the port draws natively) */
int game_FUN_0022bf94(int a0, int a1, int a2, int a3) {
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    return 0;
}

/* skyproc.c: a renderer of the console's (the port draws natively) */
void game_FUN_0022c0e0(int a0, int a1, int a2, int a3) {
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
}

/* skyproc.c: a renderer of the console's (the port draws natively) */
void game_FUN_0022c208(int a0, int a1, int a2, int a3) {
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
}

/* skyproc.c: a renderer of the console's (the port draws natively) */
void game_FUN_0022c4c8(gaddr a0, int a1, gaddr a2) {
    (void)a0;
    (void)a1;
    (void)a2;
}

/* drawquad_001FD1D8.c: a renderer of the console's (the port draws natively) */
int func_L00_001FE6D8(void) {
    return 0;
}

/* skyproc_0028D958.c: a renderer of the console's (the port draws natively) */
int func_L00_0028C680(void) {
    return 0;
}

/* tfragproc_002963D8.c: a renderer of the console's (the port draws natively) */
int func_L00_00295100(void) {
    return 0;
}

/*
 * The in-game cutscene's start (PAL func_L00_00299B68, port/game/rac1-pal/hand/level_scene_start.c),
 * written from this version's instructions (FUN_L00_00298840, 728 bytes). A scene trigger's request
 * ends here with the scene's index: it clears the scene's state (D_L00_0016C860 and five 0x40-byte
 * blocks), sets the game mode D_L00_0015F5C4 to 2, hides the HUD, flags the mobys of classes 0x4A and
 * 0xCB, fades from black, queues the scene's music track (+0x1C of the music stream state at
 * D_001516D0) and waits a frame at a time for the stream (+0x5A == 3), then runs the scene's first
 * tick and waits out its counter (+0x48, set to -3). Unlike PAL's, NTSC-U's neither tests the game
 * mode first nor saves and clears the HUD byte, and its frame counter is at +0x48 (PAL +0x46).
 *
 * Host C because the NTSC decomp has no assembly stub for it to be replaced by hand C (its level
 * code comes in whole); a stub here kept every in-game scene from starting (the level's opening
 * scene on Veldin among them).
 */
void game_FUN_L00_00298840(int index) {
    const gaddr hero = 0x0013F350u;
    const gaddr scene = OPENRAC_LDATA(0, 0x0016C860u);
    if (index < 0) {
        return;
    }
    if (GREF(int, hero + 0x22A8) == 0) {
        game_FUN_L00_00211250();
        return;
    }
    game_music_pause(0);
    game_FUN_002168a8(0);
    GREF(int, OPENRAC_LDATA(0, 0x0015F5D8u)) = 1;
    func_001F97E8(scene, 0, 0x1C0);
    func_001F97E8(OPENRAC_LDATA(0, 0x0017C440u), 0, 0x40);
    func_001F97E8(OPENRAC_LDATA(0, 0x0017C340u), 0, 0x40);
    func_001F97E8(OPENRAC_LDATA(0, 0x0017C380u), 0, 0x40);
    func_001F97E8(OPENRAC_LDATA(0, 0x0017C400u), 0, 0x40);
    func_001F97E8(OPENRAC_LDATA(0, 0x0017C3C0u), 0, 0x40);
    const gaddr header = OPENRAC_LDATA(0, 0x00173E00u);
    const int scratch = GREF(int, OPENRAC_LDATA(0, 0x001611CCu)) - 0x3C000;
    GREF(int, scene + 0x30) = index;
    GREF(int, scene + 0x5C) = GREF(int, header + 8) + scratch;
    GREF(int, scene + 0x58) = GREF(int, header + 4) + scratch;
    GREF(float, OPENRAC_LDATA(0, 0x0015F59Cu)) = 1.0f;
    GREF(int, OPENRAC_LDATA(0, 0x0015F5A0u)) = 0;
    GREF(int, scene + 0x34) = 0;
    GREF(int, scene + 0x3C) = 0;
    GREF(int, OPENRAC_LDATA(0, 0x001611CCu)) = scratch;
    game_FUN_L00_002039a0();
    GREF(float, OPENRAC_LDATA(0, 0x0015F3FCu)) = 1.0f;
    GREF(int, OPENRAC_LDATA(0, 0x0015F5C4u)) = 2;
    GREF(int, OPENRAC_LDATA(0, 0x0015F400u)) = 0;
    game_FUN_L00_002223f8(0x64, 2);
    GREF(uint8_t, hero + 0x20A5) = 1;
    game_FUN_L00_00232fe8();
    const gaddr trigger = GREF(gaddr, OPENRAC_LDATA(0, 0x00179108u));
    if (trigger != 0) {
        GREF(uint16_t, trigger + 0x34) |= 1;
    }
    const gaddr other = GREF(gaddr, OPENRAC_LDATA(0, 0x001B9DDCu));
    if (other != 0) {
        GREF(uint16_t, other + 0x34) |= 1;
    }
    game_FUN_L00_002451b8(0);
    game_FUN_001f4a58(game_FUN_001f96f8(6));
    game_FUN_002168a8(1);
    game_FUN_00204790(0);
    for (gaddr p = GREF(gaddr, OPENRAC_LDATA(0, 0x0015FFDCu)); GREF(uint8_t, p + 0x20) != 0xFF; p += 0x100) {
        const int16_t cls = GREF(int16_t, p + 0xA6);
        if (!(GREF(uint8_t, p + 0x20) & 0x80) && (cls == 0x4A || cls == 0xCB)) {
            GREF(uint16_t, p + 0x34) |= 0x80;
        }
    }
    const gaddr music = 0x001516D0u;
    GREF(int, music + 0x1C) = GREF(int, scene + 0x30);
    while (GREF(int16_t, music + 0x5A) != 3) {
        game_FUN_0022ca50();
        game_sceGsSyncV(0);
    }
    GREF(int16_t, scene + 0x48) = -3;
    game_FUN_00215b10();
    game_FUN_L00_002451b8(1);
    for (int i = 0; GREF(int16_t, scene + 0x48) < i; --i) {
        game_sceGsSyncV(0);
    }
}
/* moby_anim_eval_chain (hand-written VU0, 2640 bytes): the pose matrices of the joints marked in
 * a1 for the moby a0, into the scratchpad (MobyGetBoneMatrix reads them: the menu panels' corners,
 * items in Ratchet's hands). The port's evaluator (viewer/moby_pose, after ReRAC's evaluate_chains;
 * post-scale records are not skipped yet as the chain form does). */
void game_FUN_002109b8(gaddr a0, gaddr a1) {
    openrac_game_moby_chain(a0, a1);
}
