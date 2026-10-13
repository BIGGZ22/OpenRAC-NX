/* Carried from the PAL decompilation's nonmatching/game/func_L00_00246EC0.c (games/rac1/pal) through the version
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
/* Level-start map setup (an earlier worker's C, build-sn/fill/FUN_L00_002464a8; the buffer address
   now taken after the file pointer is read): fills the map's colour/fade tables, derives the 19 per-level
   world-to-map scale/offset pairs, then (when this level has a map) streams the map file
   in, unpacks its two parts and allocates the reveal buffers, and marks the cells of the
   32x32 reveal mask that lie outside the radius-16 disc. */
extern char D_L00_001841F0[];
extern int D_L00_00183EF0[];
extern float D_L00_00182810[];
extern float D_L00_00182A70[];
extern unsigned char D_L00_00184394[];
extern int D_L00_00173E08;
extern char D_L00_0015FE00[];
extern char D_L00_001E8B00[];
extern int D_0015ED84;
extern char D_0013D50F_m[] __asm__("D_0013D407");
extern char D_00137C80_m[] __asm__("D_00137B80");
extern char D_0014171B_m[] __asm__("D_0014161B");
extern int func_002176C8_m(int, int, int) __asm__("func_00216828");
extern int func_00234158_m(int, int, int, int) __asm__("func_00232E40");
extern int func_0020C468_m(int, int) __asm__("func_0020B618");
extern char *func_001FFAB8_m(int, int, char *, int) __asm__("func_001FF288");
extern void func_001F9A98_m(void *, void *, int) __asm__("func_001F98D0");
extern void func_002083E0_m(void *, unsigned char *, int) __asm__("func_00207BB0");
extern void func_00206F40_m(void *, int, int) __asm__("func_00206710");

void FUN_L00_002464a8(void) {
    char *base = D_L00_001841F0;
    int *src;
    float *s;
    float *d;
    int *entry;
    int level;
    int i, j;

    *(int *)(base + 0x10) = 1;
    src = D_L00_00183EF0;
    {
        int *w = (int *)(base + 0x154);
        for (i = 19; i >= 0; i--) {
            w[-20] = src[0];
            w[0] = src[1];
            src += 2;
            ((float *)w)[-40] = 0.65f;
            w++;
        }
    }

    level = D_0015ED84;
    *(int *)(base + 0x22C) = -1;
    if (level > 0x12) {
        level = 0;
    }
    *(int *)(base + 0x224) = level;

    s = D_L00_00182810;
    d = D_L00_00182A70;
    for (i = 0; i < 19; i++) {
        float x0 = s[0], y0 = s[1], x1 = s[2], y1 = s[3];
        float u0 = s[4], v0 = s[5], u1 = s[6], v1 = s[7];
        float sx = (u0 - u1) / (x0 - x1);
        float sy = (v0 - v1) / (y0 - y1);
        d[1] = sx;
        d[3] = sy;
        d[0] = u0 - sx * x0;
        d[2] = v0 - sy * y0;
        s += 8;
        d += 4;
    }

    if (*(unsigned char *)(D_0013D50F_m + 0xB9 + 0x21) != 0) {
        *(int *)(base + 0x230) = 1;
        entry = (int *)(D_00137C80_m + 0x8B8 + *(int *)(base + 0x224) * 8);
    } else {
        *(int *)(base + 0x230) = 0;
        entry = (int *)(D_00137C80_m + 0x820 + *(int *)(base + 0x224) * 8);
    }

    if (entry[1] == 0) {
        *(int *)(base + 0x0) = 0;
        *(int *)(base + 0x28) = 0;
        *(int *)(base + 0x8) = 0;
        *(int *)(base + 0xC) = 0;
        *(int *)(base + 0x4) = 0;
        return;
    }

    {
        int file;
        int sectors = entry[1];
        int buf;
        int n;
        char *p;
        unsigned char *rec;

        *(int *)(base + 0x28) = 1;
        file = D_L00_00173E08;
        buf = file + 0x9A800;
        func_002176C8_m(buf, entry[0], sectors);
        n = func_00234158_m(buf, sectors << 7, 0x4800, (int)D_L00_001E8B00);
        *(int *)(base + 0x22C) = n;
        *(int *)(base + 0x234) = sectors << 7;
        func_0020C468_m(buf, file);

        n = ((int *)file)[1] - ((int *)file)[0];
        p = func_001FFAB8_m(n, 0, D_L00_0015FE00, 0x2DB);
        *(char **)(base + 0x0) = p;
        func_001F9A98_m(p, (char *)file + ((int *)file)[0], n);
        p = *(char **)(base + 0x0) + 8;
        *(char **)(base + 0x4) = p;
        *(char **)(base + 0x0) = p;

        n = ((int *)file)[2] - ((int *)file)[1];
        p = func_001FFAB8_m(n, 0, D_L00_0015FE00, 0x2EC);
        *(char **)(base + 0x14) = p;
        func_001F9A98_m(p, (char *)file + ((int *)file)[1], n);

        p = func_001FFAB8_m(0x1000, 0, D_L00_0015FE00, 0x2F3);
        *(char **)(base + 0x8) = p;
        {
            int *q = (int *)(base + 0x38);
            for (i = 0; i < 8; i++) {
                q[-2] = -1;
                q[-1] = -1;
                q[0] = *(int *)(base + 0x8) + (i << 9);
                q += 4;
            }
        }

        p = func_001FFAB8_m(0x8000, 0, D_L00_0015FE00, 0x2FC);
        *(char **)(base + 0xC) = p;
        rec = (unsigned char *)(D_0014171B_m + 0x8A5 + (*(int *)(base + 0x224) << 11));
        if (*rec != 0) {
            func_002083E0_m(p, rec, *(int *)(base + 0x14));
        } else {
            func_00206F40_m(p, *(int *)(base + 0x0), *(int *)(base + 0x4));
        }
    }

    for (i = 0; i < 32; i++) {
        float di = 15.5f - (float)i;
        float di2 = di * di;
        for (j = 0; j < 32; j++) {
            float dj = 15.5f - (float)j;
            float r2 = dj * dj + di2;
            if (256.0f < r2) {
                D_L00_00184394[i * 4 + j / 8] |= (unsigned char)(1 << (j - (j / 8) * 8));
            }
        }
    }
}
