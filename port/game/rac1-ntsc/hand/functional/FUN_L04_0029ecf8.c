/* Carried from the PAL decompilation's nonmatching/functional/func_L04_0029FEC8.c (games/rac1/pal) through the version
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
/* functional: medium. level 4 (Eudora) update of the moby that keeps a swarm of class 0x55 mobys around it
 * (written from the retail instructions of PAL's FUN_L04_0029ecf8, 1476 bytes; not a match).
 *
 * data (the moby's variables, moby + 0x78):
 *   +0x00 home position (a quadword), +0x10 .. the swarm: data[0x90] pointers to the children,
 *   +0x90 swarm size, +0x94 "spawned" flag, +0x98 orbit angle, +0x9C orbit radius,
 *   +0xA0 path index (-1: orbit the home position), +0xA4 position on the path,
 *   +0xA8 number of class 0x55 mobys alive, counted once.
 * A path index of -1 moves the moby on a circle round its home; any other index walks it along
 * the path D_L04_001B0630[index] at a speed that grows when a child is within 3 units of it.
 * The first call spawns the swarm (random direction, distance and size); every later call steers
 * the children (func_L04_001F30E0, func_L04_001F3A80) with the steering record D_L04_00166130.
 *
 * The retail calls rely on registers the callees read: func_L04_001F30E0 gets the child, its
 * variables and the steering record as its third to fifth arguments, which the instructions
 * before the call leave in a2, a3 and t0; they are passed here. */
extern float func_001FA888_9FEC8(int) __asm__("func_001FA6C0");
extern float func_001FA748_9FEC8(float, float) __asm__("func_001FA580");
extern float func_001F9F90_9FEC8(float) __asm__("func_001F9DC8");
extern float func_001F9FA8_9FEC8(float) __asm__("func_001F9DE0");
extern float func_002140F8_9FEC8(float, float) __asm__("func_002132A8");
extern float func_00214158_9FEC8(void) __asm__("func_00213308");
extern char *func_0020D348_9FEC8(int) __asm__("func_0020C4F8");
extern float func_001F9D48_9FEC8(void *, void *) __asm__("func_001F9B80");
extern void func_001F9BC0_9FEC8(void *) __asm__("func_001F99F8");
extern int func_L04_0029FCF0_9FEC8(void *, void *, int, int, float) __asm__("func_L04_0029EB20");
extern void func_L04_001F3010_9FEC8(void *, int, void *, void *, void *) __asm__("func_L04_001F30E0");
extern void func_L04_001F39B0_9FEC8(void *, void *, void *, float) __asm__("func_L04_001F3A80");
extern void func_L00_00251E30_9FEC8(void *) __asm__("func_L00_00250DF8");
extern int D_L04_00161450_9FEC8 __asm__("D_L04_001613C0");
extern float D_L04_001661B0_9FEC8[8] __asm__("D_L04_00166130");
extern void *D_L04_001B0930_9FEC8[] __asm__("D_L04_001B0630");
extern char *D_L04_00160064_9FEC8 __asm__("D_L04_0015FFE4");
extern float D_0015EE60_9FEC8 __asm__("D_0015ED60");
extern float D_0015EE64_9FEC8 __asm__("D_0015ED64");

void FUN_L04_0029ecf8(char *moby) {
    char *data = *(char **)(moby + 0x78);
    float *g = D_L04_001661B0_9FEC8;
    char *child;
    char *cdata;
    char *p;
    float nearest;
    float approach;
    float a;
    float b;
    float r;
    float s;
    float scale;
    int i;
    int n;

    if (D_L04_00161450_9FEC8 == 0) {
        /* the steering record's constant fields, once */
        g[0] = 0.0f;
        g[2] = 0.0f;
        g[3] = 2.0f;
        g[4] = 1.0f;
        g[5] = 1.0f;
        g[6] = 0.0f;
        g[7] = 0.0f;
        g[1] = func_001FA888_9FEC8(*(int *)(data + 0xA8)) * 2.073f;
        D_L04_00161450_9FEC8 = 1;
    }

    if (*(int *)(data + 0x94) == 0) {
        *(float *)(data + 0x00) = *(float *)(moby + 0x10);
        *(float *)(data + 0x04) = *(float *)(moby + 0x14);
        *(float *)(data + 0x08) = *(float *)(moby + 0x18);
        *(float *)(data + 0x0C) = *(float *)(moby + 0x1C);
        *(int *)(data + 0x98) = 0;
    } else if (*(int *)(data + 0xA8) == 0) {
        for (p = D_L04_00160064_9FEC8; p != 0; p = *(char **)(p + 0x28)) {
            if (*(signed char *)(p + 0x20) >= 0 && *(short *)(p + 0xA6) == 0x55) {
                *(int *)(data + 0xA8) = *(int *)(data + 0xA8) + 1;
            }
        }
        g[1] = func_001FA888_9FEC8(*(int *)(data + 0xA8)) * 2.073f;
    }

    if (*(int *)(data + 0xA0) == -1) {
        /* a circle round the home position */
        *(float *)(data + 0x98) =
            func_001FA748_9FEC8(*(float *)(data + 0x98), D_0015EE60_9FEC8 * 0.0034906587f);
        *(float *)(moby + 0x10) =
            *(float *)(data + 0x00) + *(float *)(data + 0x9C) * func_001F9F90_9FEC8(*(float *)(data + 0x98));
        *(float *)(moby + 0x14) =
            *(float *)(data + 0x04) + *(float *)(data + 0x9C) * func_001F9FA8_9FEC8(*(float *)(data + 0x98));
    } else if (*(int *)(data + 0x94) != 0) {
        /* along the path, faster when a child is near */
        nearest = 10000.0f;
        for (i = 0; i < *(int *)(data + 0x90); i++) {
            a = func_001F9D48_9FEC8(moby + 0x10, *(char **)(data + 0x10 + i * 4) + 0x10);
            if (a < nearest) {
                nearest = a;
            }
        }
        approach = 0.0f;
        if (nearest < 3.0f) {
            approach = 3.0f - nearest;
        }
        *(int *)(data + 0xA4) = func_L04_0029FCF0_9FEC8(moby, D_L04_001B0930_9FEC8[*(int *)(data + 0xA0)],
                                                        *(int *)(data + 0xA4), 1, approach);
    }

    n = *(int *)(data + 0x90);
    if (*(int *)(data + 0x94) == 0) {
        /* the first call: spawn the swarm */
        *(int *)(data + 0x94) = 1;
        *(int *)(data + 0xA8) = 0;
        for (i = 0; i < n; i++) {
            *(int *)(data + 0x10 + i * 4) = 0;
            child = func_0020D348_9FEC8(0x55);
            if (child == 0) {
                continue;
            }
            cdata = *(char **)(child + 0x78);
            *(int *)(data + 0x10 + i * 4) = (int)child;
            *(unsigned char *)(child + 0x30) = 0x7F;
            *(short *)(child + 0x32) = 0x7F;
            a = func_002140F8_9FEC8(0.7f, 1.3f);
            *(float *)(child + 0x2C) = *(float *)(child + 0x2C) * a;
            *(float *)(child + 0x48) = func_00214158_9FEC8();
            *(float *)(child + 0x58) = func_002140F8_9FEC8(0.6f, 1.2f);
            func_001F9BC0_9FEC8(cdata + 0x10);
            *(short *)(cdata + 0x44) = 0;
            *(float *)(cdata + 0x4C) = 10.0f;
            *(float *)(child + 0x10) = *(float *)(moby + 0x10);
            *(float *)(child + 0x14) = *(float *)(moby + 0x14);
            *(float *)(child + 0x18) = *(float *)(moby + 0x18);
            *(float *)(child + 0x1C) = *(float *)(moby + 0x1C);
            a = func_00214158_9FEC8();
            b = func_00214158_9FEC8();
            r = func_002140F8_9FEC8(0.6f, 0.9f);
            s = func_001F9F90_9FEC8(a) * func_001F9FA8_9FEC8(b) * r;
            *(float *)(child + 0x10) = *(float *)(child + 0x10) + s;
            s = func_001F9FA8_9FEC8(a) * func_001F9FA8_9FEC8(b) * r;
            *(float *)(child + 0x14) = *(float *)(child + 0x14) + s;
            *(float *)(child + 0x18) = *(float *)(child + 0x18) + func_001F9F90_9FEC8(b) * r;
            *(int *)(cdata + 0x50) = 0;
            *(int *)(cdata + 0x54) = (int)(moby + 0x10);
            *(int *)(cdata + 0x58) = 0;
            *(float *)(cdata + 0x70) = 1.694f;
            *(float *)(cdata + 0x74) = 1.694f;
            *(float *)(cdata + 0x94) = 5.0f;
            *(float *)(cdata + 0x48) = 1.056f;
            *(short *)(cdata + 0x5C) = -1;
            *(float *)(cdata + 0x40) = 0.003f;
            *(short *)(cdata + 0x5E) = 2;
            *(short *)(cdata + 0x60) = 0x55;
            *(float *)(cdata + 0x90) = 1.0f;
            *(short *)(cdata + 0x62) = 0x1EE;
            func_L00_00251E30_9FEC8(child);
        }
    } else {
        /* every later call: steer the children */
        for (i = 0; i < n; i++) {
            child = *(char **)(data + 0x10 + i * 4);
            cdata = *(char **)(child + 0x78);
            scale = *(float *)(data + 0x9C) * 7.753f;
            a = (*(float *)(child + 0x58) + 1.0f) * 0.5f;
            g[6] = scale / 1800.0f * D_0015EE60_9FEC8 * a * 0.05f * D_0015EE64_9FEC8 * a;
            g[7] = *(float *)(data + 0x9C) * 7.753f / 1800.0f * D_0015EE60_9FEC8 * a;
            func_L04_001F3010_9FEC8(data + 0x10, *(int *)(data + 0x90), child, cdata, g);
            func_L04_001F39B0_9FEC8(child, cdata, g, 0.0f);
        }
    }
}
