/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002D3B20.c (games/rac1) through the version
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
    s16 start; /* 00 */
    s16 count; /* 02 */
    int idx; /* 04 */
    char pad08[0x18];
    float f20, f24, f28; /* 20 24 28 */
    s16 h2C, h2E; /* 2C 2E */
    int x30, x34; /* 30 34 */
    int col; /* 38 */
    u8 c0, c1, c2; /* 3C 3D 3E */
    u8 pad3F;
    s16 extra; /* 40 */
    s16 flag42; /* 42 */
    float f44, f48, pad4C, f50;
} Q_c2d2670;
typedef struct M2d1e80_c2d2670 { u8 p0[0x20]; u8 state; u8 p21[0x10]; u8 vis; u8 p32[2]; u16 flags; u8 p36[0x42]; s32 *vars; u8 p7c[0x40]; u8 bc; } M2d1e80_c2d2670;
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_c2d2670;
typedef struct { u8 pad[0x10]; Vec4f_c2d2670 pos; u8 state; u8 pad21[0xF]; u8 b30; u8 pad31[0x47]; Q_c2d2670 *q; } P_c2d2670;
extern void func_0020DAF8_2d3330(M2d1e80_c2d2670 *, s32, s32 *) __asm__("func_0020CCA8");
void func_001F9D20(Vec4f_c2d2670 *, Vec4f_c2d2670 *, void *);
void func_0020D678_D83D8(P_c2d2670 *) __asm__("func_0020C828");
extern void func_0022DA68(int, int, int);

/* NON_MATCHING FUN_L00_002d2670 -- src/overlays/shared/vendor_002D1168.c
 * Best so far: SIZE ours 1040 / retail 1052, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Decoy glove update (moby class 562): a state machine on moby[0x20] with the data block at moby[0x78]; the call
 *   Left: ours saves $fp and $s7 as well as retail's seven saved regs (retail keeps d+0x40 in $18 then $21, and G'
 *   Unblock: a second form for the data block and the G base that gives retail's saved-register set, then the sche
 */
extern void func_L00_0024F7C8(void *, int, void *);
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_0025F800(void *, void *);
extern int func_L00_002C35C0(void *, void *, void *);
extern void func_L00_002223F8(int, int);
extern int func_001F9740(int *);
extern int func_001F9688(int);
extern int func_L00_00233E98(int);
extern int func_L00_00216DE8(int, int);
extern int func_L00_00233DB8(int, int);
extern void func_L00_002D2158(void *, void *, void *);
extern void func_L00_002C3888(float *, char *, void *);
extern short D_L00_001B0830[];
extern int D_0015EEA4 MACRO_ADDR;
extern int D_0015ED84 MACRO_ADDR;
extern char D_0013E533[];
extern char D_0013A4E0[];
extern char D_0014161B[];
typedef int u128_D3B20_c2d2670 __attribute__((mode(TI)));

/* Decoy glove update: state machine on moby[0x20], per-object data block at moby[0x78]. Adapted from the NTSC decomp (MIT) for PAL: none. */
void FUN_L00_002d2670(unsigned char *moby) {
    char *d;
    char *p;
    char *g;
    char *e;
    float vb[4];
    float va[4];
    float vc[4];
    float vd[4];
    u128_D3B20_c2d2670 loc;

    if (moby == 0) return;
    d = *(char **)(moby + 0x78);
    if (d == 0) return;
    *(u128_D3B20_c2d2670 *)va = 0;
    va[1] = -0.19f;
    va[2] = -0.11f;
    func_L00_0024F7C8(moby, 0, vb);
    e = d + 0x40;
    func_0020DAF8_2d3330(moby, 0, vd);
    func_001F9D20(vc, va, vd);
    func_001F9A10(e, vb, vc);

    p = *(char **)(d + 0x50);
    if (p) {
        *(int *)(p + 0x98) = 1;
        if (*(unsigned char *)(D_0013E533 + 6) && *(short *)(*(char **)(d + 0x50) + 0xA6) != 0x76C) {
            qzero(&loc);
            func_L00_0025F800(*(char **)(d + 0x50), D_L00_001B0830);
            func_0020D678_D83D8(*(char **)(d + 0x50));
            *(int *)(d + 0x50) = func_L00_002C35C0(moby, e, &loc);
        }
    }
    if (*(char **)(d + 0x50)) {
        if (*(unsigned char *)(*(char **)(d + 0x50) + 0x20) == 0xFE || *(unsigned char *)(*(char **)(d + 0x50) + 0x20) == 0xFD) *(int *)(d + 0x50) = 0;
    }

    g = D_0013E533 + 0xE1D;
    if (*(int *)(g + 0x2084) == 1) func_L00_002223F8(0x1E, 1);
    if (func_001F9740((int *)(d + 0x54))) {
        if (*(unsigned char *)(g + 0x20AC) == 0) {
            if (*(unsigned char *)(g + 0x20A8) && *(int *)(g + 0x1BC) == func_001F9688(0x11)) goto L3D10;
            if (*(int *)(D_0013A4E0 + 0x2604) & *(int *)(g + 0x10A0)) {
                if (*(int *)(g + 0x2084) == 0x1E && func_L00_00233E98(-1)) goto L3D10;
            }
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
    func_L00_002D2158(moby, d, &loc);

    switch (moby[0x20]) {
    case 0:
        *(int *)(d + 0x50) = 0;
        moby[0x20] = (moby[0x70] & 2) ? 2 : 1;
    case 1:
        if (moby[0x70] & 2) moby[0x20] = 2;
        break;
    case 2:
        break;
    case 3:
        p = *(char **)(d + 0x50);
        if (p) {
            char *P_c2d2670 = D_0014161B + 0x65;
            if (*(unsigned short *)(P_c2d2670 + 0xC8) <= 0xFFFE) *(unsigned short *)(P_c2d2670 + 0xC8) = *(unsigned short *)(P_c2d2670 + 0xC8) + 1;
            if ((int)*(unsigned short *)(P_c2d2670 + 0xCA) < func_001F9688(D_0015EEA4) / 600)
                *(unsigned short *)(P_c2d2670 + 0xCA) = func_001F9688(D_0015EEA4) / 600;
            *(unsigned int *)(P_c2d2670 + 0xCC) = (*(unsigned int *)(P_c2d2670 + 0xCC) | (1 << D_0015ED84)) | 0x80000000;
            func_L00_002C3888((float *)e, p, *(char **)(p + 0x78) + 0x10);
            *(int *)(d + 0x50) = 0;
        } else {
            func_0022DA68(0, 0, (int)moby);
        }
        moby[0x20] = 4;
        *(int *)(d + 0x54) = func_001F9688(0x14);
        break;
    case 4:
        if (*(int *)(g + 0x2084) != 0x23 && *(unsigned char *)(g + 0x20A8) == 0) moby[0x20] = 2;
        break;
    case 5:
        moby[0x20] = 6;
        break;
    case 6:
        return;
    default:
        break;
    }

    p = *(char **)(d + 0x50);
    if (p) {
        qcopy(p + 0x10, e);
        return;
    }
    if (func_L00_00233E98(-1) || *(unsigned char *)(g + 0x20A8)) {
        qzero(&loc);
        *(int *)(d + 0x50) = func_L00_002C35C0(moby, e, &loc);
    }
}
