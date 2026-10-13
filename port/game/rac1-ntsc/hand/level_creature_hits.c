/* Carried from port/game/rac1-pal/hand/level_creature_hits.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Not yet checked in play. */
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native PAL FUN_L00_002d5160 (0x364 bytes): what the horny toad of Veldin (class
 * 749) does every tick before its states: it takes the hit record aimed at it, and
 * a hit that counts throws it off (state 12, the knocked-back flight) and takes its
 * health; then its senses (the target, the alert). Not a PS2 match.
 *
 * This is the decompilation's functional C for the function
 * (nonmatching/game/FUN_L00_002d5160.c) with one correction: whether the hit counts
 * is the damage resolver's fifth argument (1: no hit, or one still held off by the
 * cooldown), not its return value, which is the reaction to play. The functional C
 * tested the return value, so a wrench hit (reaction 1) never hurt the creature. The
 * decompilation's near miss for the same function (nonmatching/shared/) and ReRAC's
 * notes on the US release (crates/rc-game/src/moby_update/classes/units: "out5 != 1
 * and not dying: health -= damage") both read the fifth argument. */
extern char *func_L00_0025B478_6610(void *, int, int) __asm__("func_L00_0025A420");
extern int func_L00_0025B4D0_6610(void *, void *, void *, int, int *, float *, int, int) __asm__("func_L00_0025A478");
extern void func_L00_0025BBA0_6610(void *, float *, void *, void *, int) __asm__("func_L00_0025AB48");
extern void func_L00_0025D5B0_6610(void *, void *, int, int, int, float) __asm__("func_L00_0025C558");
extern void func_L00_0025E4B0_6610(void *, void *) __asm__("func_L00_0025D458");
extern void func_L00_0025E590_6610(void *, void *) __asm__("func_L00_0025D538");
extern int func_L00_00260FB0_6610(float, void *, void *, int, int, void *, int) __asm__("func_L00_0025FF38");
extern int func_001F9938_6610(void *) __asm__("func_001F9770");
extern float func_001F9878_6610(float) __asm__("func_001F96B0");
/* What the PAL file this was written in declares before it. */
typedef struct { float x, y, z, w; } __attribute__((aligned(16))) CreatureHitVec;
extern int D_L00_0015F5C4_hits __asm__("D_L00_0015F5C4") MACRO_ADDR;
extern float D_0015ED70_hits __asm__("D_0015ED70") MACRO_ADDR;
extern float D_0015ED6C_hits __asm__("D_0015ED6C") MACRO_ADDR;
extern char D_0013E533_hits[] __asm__("D_0013E533");
extern unsigned char *D_L00_001B04B0_hits[] __asm__("D_L00_001B04B0");
extern float D_L00_00161970 MACRO_ADDR;
extern float D_L00_00161974 MACRO_ADDR;
extern float D_L00_00161978 MACRO_ADDR;
extern float D_L00_0016197C MACRO_ADDR;
extern float D_L00_00161988 MACRO_ADDR;
extern float D_L00_0016198C MACRO_ADDR;

void FUN_L00_002d5160(void *a0) {
    char *a = (char *)a0;
    char *d;
    char *b;
    char *p;
    char *q;
    CreatureHitVec vec;
    int i10;
    float f14 = 0.0f;
    float t18;
    float f20;
    float f0, f1, f2, f3, f5, f6;
    int r1, r2, v;
    unsigned short h;

    if (*(unsigned char *)(a + 0x20) == 0)
        return;
    d = *(char **)(a + 0x78);

    h = *(unsigned short *)(a + 0x34);
    if (D_L00_0015F5C4_hits == 2) {
        a[0x31] = 0;
        h |= 1;
    } else {
        a[0x31] = 1;
        h &= 0xFFFE;
    }
    *(unsigned short *)(a + 0x34) = h;

    f20 = 0.0f;
    b = func_L00_0025B478_6610(a, 0x330000, 0);
    r1 = func_L00_0025B4D0_6610(a, b, d + 0x20, 0, &i10, &f14, 0, 4);
    (void)r1;

    if (i10 != 1 && *(unsigned char *)(a + 0x20) != 12 && f14 != 0.0f) {
        f6 = D_L00_00161970 * D_0015ED70_hits;
        f5 = D_L00_00161974 * D_0015ED70_hits;
        f3 = D_L00_00161978 * D_0015ED6C_hits;
        f2 = D_L00_0016197C * D_0015ED6C_hits;
        d[0xAD] = 0;
        f0 = *(float *)(d + 0x20) - f14;
        f1 = D_0015ED6C_hits + D_0015ED6C_hits;
        *(float *)(d + 0x80) = f6;
        *(float *)(d + 0x98) = 0.75f;
        *(float *)(d + 0x84) = f5;
        *(float *)(d + 0x88) = f3;
        *(float *)(d + 0x8C) = f2;
        *(float *)(d + 0x20) = f0;
        *(float *)(d + 0xBC) = f1;
        *(int *)(d + 0x94) = 41;
        *(int *)(d + 0x90) = 512;
        *(unsigned short *)(a + 0x34) &= 0xEFFF;

        vec = *(CreatureHitVec *)(b + 0x10);
        func_L00_0025BBA0_6610(&vec, &t18, d + 0x88, d + 0x8C, 41);
        f20 = t18;
        r2 = func_00213260(2);
        func_L00_0025D5B0_6610(a, d + 0x70, r2 + 6, 1, 0, f20);
        *(float *)(d + 0xC0) = D_L00_0016198C;
        *(float *)(d + 0xC4) = D_L00_00161988;
        *(unsigned char *)(a + 0x20) = 12;
        *(unsigned char *)(d + 0x67) = 0x78;
        func_L00_0025E4B0_6610(a, d + 0x60);
        func_L00_002584A8_2D19E8((u8 *)a, 0, -1);
    }
    *(unsigned char *)(a + 0xA4) = 0xFF;
    func_L00_0025E590_6610(a, d + 0x60);

    q = d + 0x180;
    if (*(unsigned char *)(a + 0x20) == 3) {
        p = D_L00_001B04B0_hits[*(int *)(d + 0x1E4)];
        func_L00_00260FB0_6610(64.0f, a, q, 0, 0, p + 0x10, *(int *)p);
    } else {
        p = D_L00_001B04B0_hits[*(int *)(d + 0x1E0)];
        func_L00_00260FB0_6610(24.0f, a, q, 0, 0, p + 0x10, *(int *)p);
    }
    v = *(int *)(d + 0x1C0);

    if (v == 0) {
        *(int *)(d + 0x1C0) = *(int *)(D_0013E533_hits + 0x2E9D);
        *(CreatureHitVec *)q = *(CreatureHitVec *)(D_0013E533_hits + 0xE9D);
    }

    func_001F9938_6610(d + 0x276);
    if (*(int *)(d + 0x38) != 0) {
        *(short *)(d + 0x276) = (short)func_001FA898_2D3F40(
            func_001F9878_6610(func_002140F8_2D19E8(180.0f, 240.0f)));
    }
    *(int *)(d + 0x38) = 0;

    if (*(unsigned char *)(a + 0x20) == 5 || *(unsigned char *)(a + 0x20) == 8) {
        if (*(short *)(d + 0x276) != 0) {
            *(unsigned char *)(a + 0x20) = 10;
            if (*(unsigned char *)(a + 0x53) != 4) {
                func_00212F90(a, 4, 0, func_001F9688(20));
            }
        }
    }
}
