/* Carried from the PAL decompilation's nonmatching/l15_quartu/func_L15_002EDB50.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L15_002ec760 -- src/overlays/l15_quartu/vendor_002EDB50.c
 * Best so far: SIZE ours 2332 / retail 2312, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   s23 (hq11, 11 of 16 runs, not EXACT): update function of moby class 1446 on level 15: a state switch (9 cases,
 *   Remaining difference: our frame is 256 bytes, retail 240 (one 16-byte stack slot holds a spilled moby+0x10 poi
 *   s23 later runs (15 of 16 used): dropping the unused A-base local and the B-pointer hoists brought the best to 
 */
extern void func_L12_002E6B60(char *);
extern void func_L02_0025C758(char *);
extern int func_L00_002668A0(void *, void *);
extern void func_0020C828(void *);
extern int func_00214720(void *arg0, int arg1);
extern int func_L00_00266448(void *, void *);
extern void func_L00_002502A0(int);
extern void func_L00_00284E50(void *, void *);
extern int func_L01_0026E008(int, int);
extern int func_001F9688(int);
extern int func_001F9740(void *);
extern void func_L15_00208CA8(void);
extern void func_00212ED8(void *, int, int);
extern void func_L00_002607D0(int);
extern void func_L00_002664B0__NOT_CARRIED(int, int);
extern void func_L00_00216F90(void *, void *, int, int);
extern void func_L00_00298840(int);
extern void func_L00_00299460(int);
extern void func_L00_00262D38(int);
extern int func_0020B178(int, int);
extern float func_001F9B80(float *, float *);
extern float func_L00_001FF8B0(float, float);
extern float func_001FA688(float, float);
extern float func_001F9AF0(void *a);
extern float func_002132A8(float, float);
extern float func_001F96B0(float);
extern int func_001FA6D0(float);
extern float func_001FA580(float, float);
extern void func_00214DB0(void *, float, float, float);
extern void func_001F9A10(void *, void *, void *);
extern void func_001F9A28(void *dst, void *a, void *b);
extern float func_001FA5C8(float, float);
extern float func_001F9B20(void *);
extern void func_L00_002628D8(char *, char *, int, float, float);
extern char D_0013E533[];
extern char D_0014161B[];
extern char D_0013DD4D[];
extern int D_L15_0015FFD8 MACRO_ADDR;
extern int D_L15_0015F5C4 MACRO_ADDR;
extern char *D_L15_0015FFE4 MACRO_ADDR;
extern char *D_L15_001600EC MACRO_ADDR;
extern int D_0015ED84 MACRO_ADDR;
extern float D_0015ED64 MACRO_ADDR;
extern unsigned char D_0015EDB0[] MACRO_ADDR;

typedef int u128_EDB50_c2ec760 __attribute__((mode(TI)));

/* update function of the ultramech scientist (moby class 1446) on level 15 */
void FUN_L15_002ec760(char *moby) {
    char *d;
    char *b;
    char *p;
    char *base;
    char *pos;
    char *a278;
    char *a160;
    float v0[4];
    float v1[4];
    float v2[4];
    float f0;
    float f2;
    float f20;
    float f21;
    float f22;
    float f23;
    int v;

    func_L12_002E6B60(moby);
    d = *(char **)(moby + 0x78);
    if (*(int *)(d + 0x14C) == -1) {
        return;
    }
    if (((unsigned char *)D_0013E533)[0xE1D + 0x20A4] != 2) {
        base = (char *)D_L15_0015FFD8;
        *(int *)(base + (*(int *)(d + 0x14C) << 8) + 0x94) = 0;
        *(unsigned short *)(base + (*(int *)(d + 0x14C) << 8) + 0x34) |= 0x41;
    }
    if (*(unsigned char *)(moby + 0x20) == 9) {
        *(unsigned short *)(moby + 0x34) |= 0x41;
        *(unsigned char *)(moby + 0x30) = 0;
        *(int *)(moby + 0x94) = 0;
        return;
    }
    if (D_L15_0015F5C4 == 2) {
        *(int *)(moby + 0x94) = 0;
        *(unsigned short *)(moby + 0x34) |= 0x41;
    } else {
        *(int *)(moby + 0x94) = *(int *)(*(char **)(moby + 0x24) + 0x10);
        *(unsigned short *)(moby + 0x34) &= 0xFFBE;
        func_L02_0025C758(moby);
    }

    pos = moby + 0x10;
    a278 = d + 0x278;
    a160 = d + 0x160;
    switch (*(unsigned char *)(moby + 0x20)) {
    case 0:
        *(unsigned char *)(moby + 0x30) = 0xFF;
        *(unsigned char *)(moby + 0x20) = 1;
        *(float *)(d + 0xC) = 20.0f;
        func_L00_002668A0(moby, d);
        if (((unsigned char *)(D_0014161B + 0xAA35))[*(int *)(d + 0x280) + (D_0015ED84 << 4)] != 0xFF) {
            break;
        }
        if (*(unsigned char *)(D_0013DD4D + 3) != 0) {
            *(unsigned char *)(moby + 0x20) = 9;
            break;
        }
        *(unsigned char *)(moby + 0x20) = 3;
        p = D_L15_0015FFE4;
        if (p == 0) {
            break;
        }
        {
        int k1 = 0xFE;
        int k2 = 0xFD;
        for (; p != 0; p = *(char **)(p + 0x28)) {
            if (*(unsigned char *)(p + 0xB0) != *(int *)(d + 0x280)) {
                continue;
            }
            if (p == moby) {
                continue;
            }
            if (p == 0) {
                continue;
            }
            if (*(unsigned char *)(p + 0x20) == k1) {
                continue;
            }
            if (*(unsigned char *)(p + 0x20) == k2) {
                continue;
            }
            func_0020C828(p);
        }
        }
        break;
    case 1:
        b = D_0013E533 + 0xE9D;
        if (func_00214720(b, *(int *)(d + 0x27C)) == 0) {
            break;
        }
        v = *(int *)(b + 0x200C);
        if ((unsigned int)v >= 2 && v != 9) {
            break;
        }
        if (func_L00_00266448(moby, d) == 0) {
            break;
        }
        func_L00_002502A0(*(int *)(d + 0x280));
        *(int *)(moby + 0x58) = 0;
        *(unsigned char *)(moby + 0x20) = 2;
        p = D_L15_0015FFE4;
        if (p == 0) {
            break;
        }
        {
        int k1 = 0xFE;
        int k2 = 0xFD;
        for (; p != 0; p = *(char **)(p + 0x28)) {
            if (*(unsigned char *)(p + 0xB0) != *(int *)(d + 0x280)) {
                continue;
            }
            if (p == moby) {
                continue;
            }
            if (p == 0) {
                continue;
            }
            if (*(unsigned char *)(p + 0x20) == k1) {
                continue;
            }
            if (*(unsigned char *)(p + 0x20) == k2) {
                continue;
            }
            func_0020C828(p);
        }
        }
        break;
    case 2:
        if (D_L15_0015F5C4 == 2) {
            break;
        }
        *(float *)(moby + 0x58) = 1.0f;
        if (*(short *)(d + 0x4) != 0) {
            *(unsigned char *)(moby + 0x20) = 9;
            break;
        }
        if (*(int *)(d + 0x150) != -1) {
            base = D_L15_001600EC;
            func_L00_00284E50(base + (*(int *)(d + 0x150) << 7) + 0x30, base + (*(int *)(d + 0x150) << 7) + 0x70);
        }
        *(unsigned char *)(moby + 0x20) = 3;
        break;
    case 3:
    if (((unsigned char *)D_0013E533)[0xE1D + 0x20A4] != 2) {
            break;
        }
        if (*(int *)(d + 0x158) != -1) {
            p = (char *)D_L15_0015FFD8 + (*(int *)(d + 0x158) << 8);
            if (p != 0) {
                v = *(unsigned char *)(p + 0x20);
                if (v != 0xFE && v != 0xFD) {
                    func_0020C828(p);
                    *(int *)(d + 0x158) = -1;
                }
            }
        }
        if (func_L01_0026E008(*(int *)(d + 0x144), 0x40) != 0) {
            break;
        }
        if (func_L01_0026E008(*(int *)(d + 0x148), 0x40) != 0) {
            break;
        }
        *(unsigned char *)(moby + 0x20) = 4;
        *(int *)(d + 0x15C) = func_001F9688(120);
        break;
    case 4:
        if (func_001F9740(d + 0x15C) == 0) {
            break;
        }
        func_L15_00208CA8();
        func_00212ED8((char *)D_L15_0015FFD8 + (*(int *)(d + 0x14C) << 8), 0, 0);
        *(short *)(d + 0x36) = 1;
        *(unsigned char *)(d + 0x8) = 1;
        *(int *)(d + 0x140) = 1;
        func_L00_002607D0(0x10);
        func_L00_002502A0(*(int *)(d + 0x284));
        func_L00_002664B0__NOT_CARRIED(0, 5);
        if (*(int *)(d + 0x154) != -1) {
            base = D_L15_001600EC;
            func_L00_00216F90(base + (*(int *)(d + 0x154) << 7) + 0x30, base + (*(int *)(d + 0x154) << 7) + 0x70, 0, 1);
            base = D_L15_001600EC;
            func_L00_00284E50(base + (*(int *)(d + 0x154) << 7) + 0x30, base + (*(int *)(d + 0x154) << 7) + 0x70);
        }
        *(unsigned char *)(moby + 0x20) = 5;
        break;
    case 5:
        func_L00_00298840(1);
        *(unsigned char *)(moby + 0x20) = 6;
        break;
    case 6:
        if (D_L15_0015F5C4 == 2) {
            break;
        }
        func_L00_00299460(0x10);
        *(unsigned char *)(moby + 0x20) = 7;
        break;
    case 7:
        if (D_L15_0015F5C4 != 2) {
            func_L00_00262D38(0x10);
            func_L00_00298840(2);
            *(unsigned char *)(moby + 0x20) = 8;
        }
        func_0020B178(0, -1);
        *(unsigned char *)(moby + 0x20) = 9;
        break;
    case 8:
        func_0020B178(0, -1);
        *(unsigned char *)(moby + 0x20) = 9;
        break;
    default:
        break;
    }

    b = D_0013E533 + 0xE9D;
    f22 = 0.02f;
    f23 = 0.3f;
    f0 = func_001F9B80((float *)pos, (float *)b);
    if (f0 < 8.0f) {
        f0 = func_L00_001FF8B0(*(float *)(b + 0x50) - *(float *)(moby + 0x10), *(float *)(b + 0x54) - *(float *)(moby + 0x14));
        f0 = func_001FA688(*(float *)(moby + 0x48), f0);
        if (f0 < 1.5707963f) {
            f0 = func_001F9AF0(b + 0x80);
            if (0.01f < f0) {
                *(int *)(d + 0x274) = func_001F9688(120);
            } else {
                func_001F9740(d + 0x274);
            }
            goto e1c4;
        }
    }
    if (*(int *)(d + 0x274) != 0) {
        *(int *)(d + 0x274) = 0;
        *(u128_EDB50_c2ec760 *)(d + 0x260) = *(u128_EDB50_c2ec760 *)(D_0013E533 + 0xEED);
    }
e1c4:
    if (func_001F9740(d + 0x278) == 0) {
        goto e26c;
    }
    f21 = 0.017453292f;
    f0 = func_002132A8(180.0f, 300.0f);
    f0 = func_001F96B0(f0);
    *(int *)(d + 0x278) = func_001FA6D0(f0);
    f0 = func_002132A8(-90.0f, 90.0f);
    func_001FA580(*(float *)(moby + 0x48), f0 * f21);
    f0 = func_002132A8(0.0f, 30.0f);
    f20 = f0;
    func_00214DB0(d + 0x260, 6.0f, f20, f0 * f21);
    func_001F9A10(d + 0x260, d + 0x260, moby + 0x10);
e26c:
    if (*(int *)(d + 0x274) != 0) {
        *(u128_EDB50_c2ec760 *)v0 = *(u128_EDB50_c2ec760 *)(D_0013E533 + 0xEED);
        f22 = 0.04f;
        f23 = 0.3f;
    } else {
        *(u128_EDB50_c2ec760 *)v0 = *(u128_EDB50_c2ec760 *)(d + 0x260);
    }
    *(u128_EDB50_c2ec760 *)v1 = *(u128_EDB50_c2ec760 *)(moby + 0x10);
    v1[2] = v1[2] + 1.0f;
    func_001F9A28(v2, v0, v1);
    f0 = func_L00_001FF8B0(v2[0], v2[1]);
    f20 = func_001FA5C8(f0, *(float *)(moby + 0x48));
    f0 = func_001F9B20(v2);
    f0 = func_L00_001FF8B0(f0, v2[2]);
    f2 = -f0;
    if (f20 > 1.5707963f) {
        f20 = 1.5707963f;
    } else if (f20 < -1.5707963f) {
        f20 = -1.5707963f;
    }
    if (f2 > 0.5235988f) {
        f2 = 0.5235988f;
    } else if (f2 < -0.5235988f) {
        f2 = -0.5235988f;
    }
    *(float *)(d + 0x248) = f20 * 0.5f;
    *(float *)(d + 0x1C4) = f2 * 1.25f;
    *(float *)(d + 0x1C8) = f20 * 0.5f;
    if (D_0015EDB0[0] != 0) {
        *(float *)(d + 0x1D0) = 2.75f;
    }
    func_L00_002628D8(moby, d + 0x160, 1, f22 * D_0015ED64, f23 * D_0015ED64);
    func_L00_002628D8(moby, d + 0x1E0, 0, f22 * D_0015ED64, f23 * D_0015ED64);
}
