/* Carried from the PAL decompilation's nonmatching/game/func_L00_0020EDC0.c (games/rac1/pal) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it. */
typedef struct {
    u8 p0[0xD08];
    u8 *slot[2];
    u8 p1[0xD14 - 0xD10];
    s32 iD14;
    u8 p2[0x2080 - 0xD18];
    u8 *p2080;
    u8 p3[0x208C - 0x2084];
    u8 *w208C;
    u8 p4[0x20A4 - 0x2090];
    u8 b20A4;
    u8 p5[3];
    u8 b20A8;
    u8 b20A9;
    u8 b20AA;
    u8 b20AB;
    u8 p6[0x2294 - 0x20AC];
    s32 i2294;
    s32 i2298;
    u8 p7[0x22C8 - 0x229C];
    s16 h22C8;
} PS_20e4c8_c20e728;
typedef struct {
    u8 p0[0x18];
    s32 f18;
    u8 p1[0x24 - 0x1C];
    s32 f24;
    s32 f28;
    s32 f2C;
    s32 f30;
    u8 p2[0x4C - 0x34];
} ID_20e4c8_c20e728;
extern PS_20e4c8_c20e728 D_0013F450_0EB60 __asm__("D_0013F350");
extern void func_L00_0020E698(void);
extern int func_L00_0020D498(int);
extern ID_20e4c8_c20e728 D_L00_00179BC0_0EB60[] __asm__("D_L00_00179AC0");
extern int func_001F9688(int);
extern void func_L00_0024F3E0(int, void *, int, int, int, int);
extern void func_L00_0024F190(void *, unsigned char *);
extern float func_00213ED8(float *, float, float);
extern float D_0015ED60 MACRO_ADDR;
extern void func_L00_0024F0E8(void *, void *);
extern f32 func_L00_001FF8B0(f32, f32);
extern f32 func_001FA580(f32, f32);

/* Hero animation slots (two entries at 0xD08 of the hero block): keeps each slot's object
   animated from the table D_L00_00179AC0 for the current state, and returns early when a
   slot's distance check (byte 0x23 / float at +8) says the object is done. Finishes with
   the 0x640..0x6FF block (a copy of the object's 0xC0..0xEF data or the 0x90 block). */
extern int func_L00_00233E98(int);
extern void func_001F99F8(void *);
extern void func_001F9CF8(void *, void *, void *);
extern void func_001FA030(void *, void *);

void FUN_L00_0020e728(void) {
    char *g = (char *)&D_0013F450_0EB60;
    int k;

    if (*(unsigned char *)(g + 0x20A8) != 0 && *(unsigned char *)(g + 0x20AA) != 0) {
        if (func_L00_00233E98(-1) == 0) {
            func_L00_0020E698();
        }
    }

    for (k = 0; k < 2; k++) {
        unsigned char **slotp = (unsigned char **)(g + 0xD08) + k;
        unsigned char *p = *slotp;
        int ret;
        int d14;

        if (p == 0) {
            continue;
        }
        ret = func_L00_0020D498(0);
        if (ret == *(int *)(g + 0x2298)) {
            int v5;
            if (*(int *)(g + 0x208C) == 0xC) {
                v5 = D_L00_00179BC0_0EB60[ret].f2C;
            } else {
                v5 = D_L00_00179BC0_0EB60[ret].f28;
            }
            if (v5 != -1 && *(unsigned char *)(p + 0x23) != v5) {
                int r;
                *(int *)(g + 0x2294) = v5;
                r = func_001F9688(8);
                func_L00_0024F3E0(*(int *)(g + 0x2080), p, *(int *)(g + 0x2294), 0, r, 1);
            }
        }

        func_L00_0024F190(*(void **)(g + 0x2080), p);

        d14 = *(int *)(g + 0xD14);
        if (d14 == 0) {
            func_00213ED8((float *)(p + 8), 1.0f, D_0015ED60 * 0.2f);
            if ((p[0x5] & 2) == 0) {
                continue;
            }
            if (*(unsigned char *)(g + 0x20AA) == 0) {
                *(int *)(g + 0xD14) = 1;
                *(int *)(p + 0x28) = 0;
                *(unsigned char *)(g + 0x20A8) = 0;
                return;
            }
            continue;
        }
        if (d14 == 2) {
            func_00213ED8((float *)(p + 8), 0.0f, D_0015ED60 * 0.25f);
        } else {
            func_00213ED8((float *)(p + 8), 0.0f, D_0015ED60 * 0.07f);
        }
        if (*(float *)(p + 8) != 0.0f) {
            continue;
        }
        func_L00_0024F0E8(*(void **)(g + 0x2080), slotp);
        return;
    }

    if (*(int *)(g + 0x208C) != 0xF) {
        char *obj = *(char **)(g + 0x2080);
        char *a = g + 0x640;
        char *b = g + 0x670;
        int i;
        for (i = 0; i < 16; i++) {
            a[i] = obj[0xC0 + i];
        }
        for (i = 0; i < 16; i++) {
            a[0x10 + i] = obj[0xD0 + i];
        }
        for (i = 0; i < 16; i++) {
            a[0x20 + i] = obj[0xE0 + i];
        }
        func_001F99F8(b);
        *(float *)(g + 0x670) = 1.0f;
        func_001F9CF8(b, b, a);
        for (i = 0; i < 16; i++) {
            (g + 0x680)[i] = (g + 0x90)[i];
        }
        return;
    } else {
        char *ptr = *(char **)(g + 0x5D8);
        float f0;
        float f1;
        int i;
        char *h;

        if (ptr != 0) {
            float a1 = *(float *)(g + 0x80);
            float a2 = *(float *)(g + 0x84);
            float c12;
            float c13;
            for (i = 0; i < 16; i++) {
                (g + 0x680)[i] = (g + 0x90)[i];
            }
            c12 = *(float *)(ptr + 0x10) - a1;
            c13 = *(float *)(ptr + 0x14) - a2;
            f0 = func_L00_001FF8B0(c12, c13);
            f1 = *(float *)(g + 0x580);
        } else {
            for (i = 0; i < 16; i++) {
                (g + 0x680)[i] = (g + 0x90)[i];
            }
            f0 = func_001FA580(*(float *)(g + 0x57C), *(float *)(g + 0x5D0));
            f1 = *(float *)(g + 0x580);
        }
        *(float *)(g + 0x688) = f0;
        *(float *)(g + 0x684) = -f1;

        h = g + 0x640;
        func_001FA030(h, h + 0x40);
        func_001F99F8(h + 0x30);
        *(float *)(h + 0x30) = 1.0f;
        func_001F9CF8(h + 0x30, h + 0x30, h);
        return;
    }
}
