/* Carried from the PAL decompilation's src/overlays/shared/vendor_002D1168.c (matched C there) (games/rac1) through the version
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
/* What the PAL file this was written in declares before it (its unit, and its own file). */
extern char D_0013E533[];
extern void func_001F9A10(void *, void *, void *);
typedef struct M2d1e80_c2d2670 { u8 p0[0x20]; u8 state; u8 p21[0x10]; u8 vis; u8 p32[2]; u16 flags; u8 p36[0x42]; s32 *vars; u8 p7c[0x40]; u8 bc; } M2d1e80_c2d2670;
extern void func_0020DAF8_2d3330(M2d1e80_c2d2670 *, s32, s32 *) __asm__("func_0020CCA8");
typedef struct Moby_2D3608_c2d2670 {
    char pad0[0x48];
    float yaw;
    char pad4C[4];
    struct Moby_2D3608_c2d2670 *target;
    char pad54[0x24];
    float *data;
} Moby_2D3608_c2d2670;
void func_L00_002D2158(int collision_arg, Moby_2D3608_c2d2670 *carrier, float *position);
extern void func_L00_0024F7C8(void *, int, void *);
extern void func_L00_0025F800(void *, void *);
extern int func_L00_002C35C0(void *, void *, void *);
extern void func_L00_002223F8(int, int);
extern int func_001F9740(int *);
extern int func_001F9688(int);
extern int func_L00_00233E98(int);
extern int func_L00_00216DE8(int, int);
extern int func_L00_00233DB8(int, int);
extern void func_L00_002C3888(float *, char *, void *);
extern short D_L00_001B0830[];
extern int D_0015EEA4 MACRO_ADDR;
extern int D_0015ED84 MACRO_ADDR;
extern char D_0013A4E0[];
extern char D_0014161B[];
typedef int u128_D3B20_c2d2670 __attribute__((mode(TI)));



/* Decoy glove update: state machine on moby[0x20], per-object data block at moby[0x78]. Adapted from Lombyte (MIT) for PAL: none. */
void FUN_L00_002d2670(unsigned char *moby) {
    extern void func_0020C828(void);
    extern void func_001F9D20();
    extern void func_0022DA68(int, int, int);
    char *d;
    char *p;
    char *g;
    float vb[4];
    float va[4];
    float vc[4];
    float vd[4];
    float pad0[4];
    float pad1[4];
    float pad2[4];
    u128_D3B20_c2d2670 loc;

    if (moby == 0) return;
    d = *(char **)(moby + 0x78);
    if (d == 0) return;
    *(u128_D3B20_c2d2670 *)va = 0;
    va[1] = -0.19f;
    va[2] = -0.11f;
    func_L00_0024F7C8(moby, 0, vb);
    func_0020DAF8_2d3330(moby, 0, vd);
    func_001F9D20(vc, va, vd);
    func_001F9A10(d + 0x40, vb, vc);

    p = *(char **)(d + 0x50);
    if (p) {
        *(int *)(p + 0x98) = 1;
        if (*(unsigned char *)(D_0013E533 + 6) && *(short *)(*(char **)(d + 0x50) + 0xA6) != 0x76C) {
            qzero(&loc);
            func_L00_0025F800(*(char **)(d + 0x50), D_L00_001B0830);
            ((void (*)(void *))func_0020C828)(*(char **)(d + 0x50));
            *(int *)(d + 0x50) = func_L00_002C35C0(moby, d + 0x40, &loc);
        }
        if (*(char **)(d + 0x50) != 0 &&
            *(unsigned char *)(*(char **)(d + 0x50) + 0x20) != 0xFE &&
            *(unsigned char *)(*(char **)(d + 0x50) + 0x20) != 0xFD)
            goto target_valid;
    }
    *(int *)(d + 0x50) = 0;
target_valid:
    g = D_0013E533 + 0xE1D;
    if (*(int *)(g + 0x2084) == 1) func_L00_002223F8(0x1E, 1);
    if (func_001F9740((int *)(d + 0x54))) {
        if (*(unsigned char *)(g + 0x20AC) == 0) {
            if (*(unsigned char *)(g + 0x20A8) && *(int *)(g + 0x1BC) == func_001F9688(0x11)) goto L3D10;
            if (*(int *)(D_0013A4E0 + 0x2604) & *(int *)(g + 0x10A0)) {
                if (*(int *)(g + 0x2084) == 0x1E && func_L00_00233E98(-1)) goto L3D10;
            }
            g = D_0013E533 + 0xE1D;
            if (*(int *)(g + 0x2084) == 0x23 && *(int *)(g + 0x198) == func_001F9688(0x10)) goto L3D10;
        }
    }
    goto L3D34;
L3D10:
    func_L00_00216DE8(0x1A, 0);
    func_L00_00233DB8(-1, 1);
    moby[0x20] = 3;
L3D34:
    *(u128_D3B20_c2d2670 *)&loc = *(u128_D3B20_c2d2670 *)(d + 0x40);
    func_L00_002D2158((int)moby, (void *)d, (float *)&loc);

    switch (moby[0x20]) {
    case 0:
        *(int *)(d + 0x50) = 0;
        if (!(moby[0x70] & 2)) moby[0x20] = 1; else moby[0x20] = 2;
    case 1:
        if (moby[0x70] & 2) moby[0x20] = 2;
        break;
    case 2:
        break;
    case 3:
        if (*(char **)(d + 0x50)) {
            char *P = D_0014161B + 0x65;
            if (*(unsigned short *)(P + 0xC8) <= 0xFFFE) *(unsigned short *)(P + 0xC8) = *(unsigned short *)(P + 0xC8) + 1;
            if (func_001F9688(D_0015EEA4) / 600 > (int)*(unsigned short *)(P + 0xCA))
                *(unsigned short *)(P + 0xCA) = func_001F9688(D_0015EEA4) / 600;
            *(unsigned int *)(P + 0xCC) = (*(unsigned int *)(P + 0xCC) | (1 << D_0015ED84)) | 0x80000000;
            func_L00_002C3888((float *)(d + 0x40), *(char **)(d + 0x50), *(char **)(*(char **)(d + 0x50) + 0x78) + 0x10);
            *(int *)(d + 0x50) = 0;
        } else {
            func_0022DA68(0, 0, (int)moby);
        }
        moby[0x20] = 4;
        *(int *)(d + 0x54) = func_001F9688(0x14);
        break;
    case 4: {
        char *h = D_0013E533 + 0xE1D;
        if (*(int *)(h + 0x2084) != 0x23 && *(unsigned char *)(h + 0x20A8) == 0) moby[0x20] = 2;
        break;
    }
    case 5:
        moby[0x20] = 6;
        break;
    case 6:
        return;
    default:
        break;
    }

    if (*(char **)(d + 0x50)) {
        qcopy(*(char **)(d + 0x50) + 0x10, d + 0x40);
        return;
    }
    if (func_L00_00233E98(-1)) {
        qzero(&loc);
        *(int *)(d + 0x50) = func_L00_002C35C0(moby, d + 0x40, &loc);
    } else {
        char *h = D_0013E533 + 0xE1D;
        if (*(unsigned char *)(h + 0x20A8)) {
            qzero(&loc);
            *(int *)(d + 0x50) = func_L00_002C35C0(moby, d + 0x40, &loc);
        }
    }
}
