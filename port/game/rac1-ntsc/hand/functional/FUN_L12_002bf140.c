/* Carried from the PAL decompilation's nonmatching/functional/func_L12_002C0310.c (games/rac1) through the version
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
/* functional: medium. level 12 moby update (class 19): state 0 claims a slot in the 0x1190-byte table and builds it,
 * state 2 steers, then the per-frame tail. Not a match (1380 bytes against 1420). Starts from the near miss in
 * nonmatching/l12_hoven/, with two differences found by tools/equiv.py: the second near-hero test stores the
 * moby's z (+0x18) like the first, not its distance; the 1/sqrt(3) constant is 0x3F13CD36 (0.57735f). */
extern char D_0013E533[];
extern char D_L12_0020BC90[];
extern short D_L12_00161388;
extern char D_L12_001CBE00[];
extern char D_L12_001CAD00[];
extern char D_L12_00180340[];
extern int D_L12_001612D0 MACRO_ADDR;
extern int D_L12_001612D4 MACRO_ADDR;
extern int D_L12_001612D8 MACRO_ADDR;
extern float D_L12_0016120C MACRO_ADDR;
extern float D_L12_00161210 MACRO_ADDR;
extern float D_L12_00161214 MACRO_ADDR;
extern int D_L12_00161208 MACRO_ADDR;
extern float D_0015ED70 MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;
extern char D_L12_00167240[];
extern char D_L12_001CBF40[];
extern int func_001E93B0();
extern void func_L01_002B7A48(void *, int);
extern void func_L01_002B7EF0(float s);
extern float func_L00_001FF8B0(float, float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern void func_L01_002B7C68(float *in, int *out);
extern void func_L12_002BF770(int);
extern float func_00213F38(float *, float *, float, float, float, float);
extern void func_L11_0030E690(char *moby);
extern float func_001F99C0(float);
extern int func_00213260(int);
extern float func_002132A8(float, float);
extern void func_L00_002A3EC8(void *, int, int, float, float, float, float);
extern int func_L01_00275690(char *vec, float scale);
extern int func_00214720();
extern void func_L01_002B7FE0(char *p, int n);
extern void func_001F4600(void *, void *);
extern void func_L12_002BF748(void);

/* Level 12 moby update (class 19): state 0 claims a slot and builds the table, state 2 steers, then the per-frame tail. */
void FUN_L12_002bf140(char *moby) {
    char *data;
    char *rec;
    char *x;
    char *p;
    float *P;
    float f20;
    float f21;
    float f;
    float g;
    float r1;
    float r2;
    int st;
    int n;
    int k;
    int idx;
    int r;

    st = *(unsigned char *)(moby + 0x20);
    data = *(char **)(moby + 0x78);
    if (st != 1) {
        if (st < 2) {
        n = *(int *)&D_L12_00161388 + 1;
        *(int *)(data + 0xC) = *(int *)&D_L12_00161388;
        *(int *)&D_L12_00161388 = n;
        if (n >= 0x80) {
            func_001E93B0(D_L12_0020BC90);
        }
        f = *(float *)(*(char **)(moby + 0x24) + 0x24);
        *(float *)(moby + 0x2C) = f + f;
        *(int *)data = 3;
        *(unsigned char *)(moby + 0x30) = 0xFF;
        *(unsigned short *)(moby + 0x34) = *(unsigned short *)(moby + 0x34) | 1;
        if (*(int *)(data + 0xC) == 0) {
            p = D_L12_001CBF40 + 0x1191E;
            k = 0x10;
            do {
                *(unsigned short *)p = 0;
                k = k - 1;
                p = p - 0x1190;
            } while (k >= 0);
            f20 = 1.0f;
            func_L01_002B7A48(D_L12_001CBF40, 0x11);
            func_L01_002B7EF0(f20);
            P = (float *)D_L12_001CAD00;
            P[0] = 16.0f;
            P[3] = -8.0f;
            P[6] = 0.9f;
            P[2] = -8.0f;
            P[8] = 0.1f;
            P[5] = 1.0f;
            P[4] = 1.0f;
            P[1] = 16.0f;
            f20 = func_L00_001FF8B0(*(float *)(D_L12_00180340 + 0x10), *(float *)(D_L12_00180340 + 0x14));
            P[12] = func_001F9DC8(f20) * 0.57735f;
            g = func_001F9DE0(f20) * 0.57735f;
            P[14] = -0.57735f;
            P[13] = g;
            *(int *)(P + 10) = *(int *)data + 0x28;
            *(unsigned char *)((char *)P + 0x3D) = 0x40;
            *(unsigned char *)((char *)P + 0x3C) = 0x1C;
            *(int *)(P + 11) = *(int *)(data + 4) + 0x28;
            *(int *)&D_L12_001612D0 = (int)D_L12_001CBF40;
            *(int *)&D_L12_001612D4 = (int)D_L12_001CBE00;
            *(int *)&D_L12_001612D8 = 0x11;
            D_L12_0016120C = 32768.0f;
            D_L12_00161210 = 255.0f;
            D_L12_00161214 = 48.0f;
            D_L12_00161208 = 0;
        }
        idx = *(int *)(data + 0xC);
        *(float *)(D_L12_001CBF40 + *(int *)(data + 0xC) * 0x1190) = *(float *)(moby + 0x10);
        *(float *)(D_L12_001CBF40 + *(int *)(data + 0xC) * 0x1190 + 4) = *(float *)(moby + 0x14);
        *(float *)(D_L12_001CBF40 + *(int *)(data + 0xC) * 0x1190 + 8) = *(float *)(moby + 0x18);
        *(int *)(D_L12_001CBF40 + *(int *)(data + 0xC) * 0x1190 + 0x14) = *(int *)data + 0x28;
        *(int *)(D_L12_001CBF40 + *(int *)(data + 0xC) * 0x1190 + 0x18) = *(int *)(data + 4) + 0x28;
        *(unsigned char *)(D_L12_001CBF40 + *(int *)(data + 0xC) * 0x1190 + 0x1C) = 0x1C;
        *(unsigned char *)(D_L12_001CBF40 + *(int *)(data + 0xC) * 0x1190 + 0x1D) = 0x44;
        *(unsigned short *)(D_L12_001CBF40 + *(int *)(data + 0xC) * 0x1190 + 0x1E) = 0xFFFF;
        func_L01_002B7C68((float *)(D_L12_001CBF40 + *(int *)(data + 0xC) * 0x1190), (int *)(D_L12_001CBE00 + *(int *)(data + 0xC) * 16));
        *(unsigned char *)(moby + 0x20) = 1;
        } else if (st == 2) {
        if (*(unsigned char *)(moby + 0xBC) & 4) {
            *(float *)(data + 0x10) = *(float *)(data + 0x10) + 5.0f;
            *(int *)(data + 0x14) = 1;
        }
        *(unsigned char *)(moby + 0xBC) = 1;
        if (*(int *)(data + 0x14) != 0) {
            func_00213F38((float *)(moby + 0x18), (float *)(data + 8), *(float *)(data + 0x10), D_0015ED70 * 4.0f, D_0015ED70 * 4.0f, D_0015ED6C * 4.0f);
            func_L11_0030E690(moby);
        }
        x = D_0013E533 + 0xE1D;
        f = func_001F99C0(*(float *)(x + 0x80) - *(float *)(moby + 0x10));
        if (f <= 8.0f) {
            *(float *)(x + 0x2F0) = *(float *)(moby + 0x18);
        } else {
            g = func_001F99C0(*(float *)(x + 0x84) - *(float *)(moby + 0x14));
            if (g <= 8.0f) {
                *(float *)(x + 0x2F0) = *(float *)(moby + 0x18);
            }
        }
        if (func_00213260(200) == 0) {
            r1 = func_002132A8(-4.0f, 4.0f);
            f20 = *(float *)(D_L12_001CBF40 + *(int *)(data + 0xC) * 0x1190) + r1;
            r2 = func_002132A8(-4.0f, 4.0f);
            func_L00_002A3EC8(D_L12_001CBF40 + *(int *)(data + 0xC) * 0x1190, 1, 1, f20, *(float *)(D_L12_001CBF40 + *(int *)(data + 0xC) * 0x1190 + 4) + r2, 1.0f, -0.05f);
        }
        }
    } else {
        func_L12_002BF770(*(int *)(data + 0xC));
        *(int *)&D_L12_00161388 = 0;
        *(unsigned char *)(moby + 0x20) = 2;
        *(float *)(data + 0x10) = *(float *)(moby + 0x18);
    }

    if (*(int *)(data + 0xC) >= 0) {
        r = func_L01_00275690(moby, 64.0f);
        f20 = 0.0f;
        if (r != -1) {
            f20 = 1.0f;
        }
        if (*(int *)(data + 0x18) != -1) {
            func_00214720(D_L12_00167240);
            f20 = 0.0f;
        }
        idx = *(int *)(data + 0xC);
        rec = D_L12_001CBF40 + idx * 0x1190;
        if (f20 == 0.0f) {
            *(unsigned short *)(D_L12_001CBF40 + *(int *)(data + 0xC) * 0x1190 + 0x1E) = 0;
        } else {
            *(unsigned short *)(D_L12_001CBF40 + *(int *)(data + 0xC) * 0x1190 + 0x1E) = 0xFFFF;
        }
        if (*(int *)(data + 0xC) == 0) {
            func_L01_002B7FE0(D_L12_001CBF40, 0x11);
            func_001F4600((void *)func_L12_002BF748, moby);
        }
    }
}
