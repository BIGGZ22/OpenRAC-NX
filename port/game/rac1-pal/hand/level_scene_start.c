/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank (PAL): the in-game cutscene's start (func_L00_00299B68), in C.
 *
 * Written from the retail routine and the decompilation's near miss
 * (games/rac1/nonmatching/shared/func_L00_00299B68.c, 780 of 772 bytes, only the register
 * allocation differs). A cutscene trigger's request (func_L00_00267290, automatic by proximity
 * for Veldin's Clank) ends here with the scene's index. It clears the scene's state
 * (D_L00_0016C960 and five 0x40-byte blocks), sets the game mode D_L00_0015F6A8 to 2 (in a
 * scene: the trigger mobys wait for it), hides the HUD, flags the mobys of types 0x4A and 0xCB
 * (bit 0x80 of +0x34, cleared again by the teardown func_L00_00299E70), fades from black, then
 * queues the scene's music track (+0x1C of the music stream state at D_0014171B + 0x100B5) and
 * waits, a frame at a time, for the stream to report it ready (+0x5A == 3). Last, it runs the
 * scene's first tick and waits out the frames until its counter at +0x46 (set to -3) reaches 0.
 *
 * The port used to stub it as a renderer (it is in tieproc_00299108.c), so no in-game scene ever
 * started: the trigger dropped back to waiting and the level never moved on.
 *
 * Refactor notes: D_L00_0016C960 is the scene state (a struct, CT in tieproc_00299108.c; +0x30
 * the index, +0x46 the frame counter, +0x4B the saved HUD byte, +0x58 and +0x5C the data
 * pointers); the gp words D_L00_0015F6BC, D_L00_0015F500 and D_L00_0016128C want names once
 * their other users are read; func_0022DD68 is the sound update and func_00122598 the vsync.
 */
#include "common.h"

extern int D_L00_0015F6A8;   /* the game mode: 2 while a scene plays */
extern int D_L00_0015F6BC;   /* set at a scene's start and end */
extern int D_L00_0015F500;
extern float D_L00_0015F4FC;
extern float D_L00_0015F67C;
extern int D_L00_0015F680;
extern int D_L00_0016128C;   /* the level's scratch allocation pointer */
extern char D_0013E633[];    /* the hero's block (+0xE1D) */
extern char D_0014171B[];    /* + 0x100B5: the music stream state */
extern char D_L00_0016C960[];  /* the scene state */
extern char D_L00_0017C540[];
extern char D_L00_0017C440[];
extern char D_L00_0017C480[];
extern char D_L00_0017C500[];
extern char D_L00_0017C4C0[];
extern int D_L00_00173F00[];   /* the scene data's header */
extern unsigned char D_L00_00166D80[];
extern char *D_L00_00179208;   /* the trigger moby of the request */
extern char *D_L00_001BA15C;
extern unsigned char *D_L00_0016009C;  /* the moby table, 0x100 bytes each, 0xFF ends it */

extern void func_L00_00211908(void);
extern void func_00216EF0(int);
extern short func_00217748(int);
extern void func_001F99B0(void *, int, int);  /* memset */
extern void func_L00_00203FB8(void);
extern int func_L00_00222B80(int, int);
extern void func_L00_00233868(void);  /* hides the HUD */
extern void func_L00_00245B88(int);
extern int func_001F9850(int);
extern void func_001F4E08(int frames, unsigned int color);  /* fade in from black */
extern void func_00204FC0(void *);
extern int func_0022DD68(void);  /* the sound update */
extern int func_00122598(int);   /* vsync: ends the frame */
extern int func_00216960(void);

void func_L00_00299B68(int index) {
    char *hero = D_0013E633 + 0xE1D;
    char *scene = D_L00_0016C960;
    char *music;
    unsigned char *p;
    int i;

    if (index < 0 || D_L00_0015F6A8 == 2) {
        return;
    }
    if (*(int *)(hero + 0x22A8) == 0) {
        func_L00_00211908();
        return;
    }
    func_00216EF0(0);
    func_00217748(0);
    D_L00_0015F6BC = 1;
    func_001F99B0(scene, 0, 0x1C0);
    func_001F99B0(D_L00_0017C540, 0, 0x40);
    func_001F99B0(D_L00_0017C440, 0, 0x40);
    func_001F99B0(D_L00_0017C480, 0, 0x40);
    func_001F99B0(D_L00_0017C500, 0, 0x40);
    func_001F99B0(D_L00_0017C4C0, 0, 0x40);
    *(int *)(scene + 0x30) = index;
    *(int *)(scene + 0x58) = D_L00_00173F00[1] + (D_L00_0016128C - 0x3C000);
    *(int *)(scene + 0x5C) = D_L00_00173F00[2] + (D_L00_0016128C - 0x3C000);
    D_L00_0015F67C = 1.0f;
    D_L00_0015F680 = 0;
    *(int *)(scene + 0x34) = 0;
    *(int *)(scene + 0x3C) = 0;
    D_L00_0016128C -= 0x3C000;
    func_L00_00203FB8();
    D_L00_0015F4FC = 1.0f;
    D_L00_0015F6A8 = 2;
    scene[0x4B] = D_L00_00166D80[0x394];
    *(int *)(D_L00_00166D80 + 0x394) = 0;
    D_L00_0015F500 = 0;
    func_L00_00222B80(0x64, 2);
    hero[0x20A5] = 1;
    func_L00_00233868();
    if (D_L00_00179208 != 0) {
        *(unsigned short *)(D_L00_00179208 + 0x34) |= 1;
    }
    if (D_L00_001BA15C != 0) {
        *(unsigned short *)(D_L00_001BA15C + 0x34) |= 1;
    }
    func_L00_00245B88(0);
    /* Retail leaves a1 as func_001F9850 left it; the fade does not read the colour. */
    func_001F4E08(func_001F9850(6), 0);
    func_00217748(1);
    func_00204FC0(0);
    for (p = D_L00_0016009C; p[0x20] != 0xFF; p += 0x100) {
        if (!(p[0x20] & 0x80) && (*(short *)(p + 0xA6) == 0x4A || *(short *)(p + 0xA6) == 0xCB)) {
            *(unsigned short *)(p + 0x34) |= 0x80;
        }
    }

    /* Queue the scene's track and wait for its stream. */
    music = D_0014171B + 0x100B5;
    *(int *)(music + 0x1C) = *(int *)(scene + 0x30);
    while (*(short *)(music + 0x5A) != 3) {
        func_0022DD68();
        func_00122598(0);
    }

    *(short *)(scene + 0x46) = -3;
    func_00216960();
    func_L00_00245B88(1);
    for (i = 0; *(short *)(scene + 0x46) < i; i--) {
        func_00122598(0);
    }
}
