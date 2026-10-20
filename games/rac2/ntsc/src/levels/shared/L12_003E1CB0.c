#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed short s16;
typedef unsigned int u32;
typedef int s32;
typedef float f32;

/* Level data. */
extern s32 D_L12_001B2E9C __attribute__((sda));
extern s32 D_L12_001A8FF0 __attribute__((sda));
extern u8 D_L12_001A63A8[];
extern u8 D_L12_001FEAC0[];
extern s16 *D_L12_001FB960[];

extern void func_331870();
extern void func_32F848();
extern void func_32F978(f32);

extern void func_L12_00322AF8(s32, s32);
extern void func_L12_00400900();
extern void func_L12_0031E1D0(s32, s32, s32);
extern void func_L12_002F15C8(s32);
extern void func_L12_00326948(s32, s32, s32, f32, f32);
extern void func_L12_003296A8(s32, s32, s32, s32, f32);

extern float func_00282E80(void *, void *);
extern int func_002761F8(void *, void *, int, int, int);
extern float func_L02_002F21C0(void *, void *);

void func_L12_003E1CB0(s32 a0) {
    u8 *state = *(u8 **)(a0 + 0x68);
    u8 *list = *(u8 **)(D_L12_001FEAC0 + (*(s32 *)(state + 0x158) << 2));
    s32 i;
    s32 bestIdx;
    f32 best;
    f32 d;

    best = func_00282E80(a0 + 0x10, list + 0x10 + (*(s32 *)(state + 0x264) << 4));
    if (best < 0.5f) {
        *(s32 *)(state + 0x16C) = 0;
        return;
    }
    if (best < 5.0f) {
        if (func_002761F8(a0 + 0x10, list + 0x10 + (*(s32 *)(state + 0x264) << 4), 2, 0, 0) == 0) {
            *(s32 *)(state + 0x16C) = 1;
            return;
        }
    }
    bestIdx = -1;
    best = 1000.0f;
    for (i = 0; i < *(s32 *)list; i++) {
        d = func_L02_002F21C0(a0 + 0x10, list + 0x10 + i * 16);
        if (d < best) {
            best = d;
            bestIdx = i;
        }
    }
    if (best < 0.5f) {
        *(s32 *)(state + 0x264) = bestIdx;
        *(s32 *)(state + 0x16C) = 0;
        return;
    }
    if (func_002761F8(a0 + 0x10, list + 0x10 + i * 16, 2, 0, 0) == 0) {
        *(s32 *)(state + 0x264) = bestIdx;
        *(s32 *)(state + 0x16C) = 1;
        return;
    }
    bestIdx = -1;
    best = 1000.0f;
    for (i = 0; i < *(s32 *)list; i++) {
        if (func_002761F8(a0 + 0x10, list + 0x10 + i * 16, 2, 0, 0) != 0)
            continue;
        d = func_L02_002F21C0(a0 + 0x10, list + 0x10 + i * 16);
        if (d < best) {
            best = d;
            bestIdx = i;
        }
    }
    if (bestIdx == -1) {
        *(s32 *)(state + 0x290) = *(s32 *)(state + 0x290) + 1;
        *(s32 *)(state + 0x16C) = 0;
    } else {
        *(s32 *)(state + 0x264) = bestIdx;
        *(s32 *)(state + 0x16C) = 1;
    }
}

INCLUDE_ASM("asm/levels", func_L12_003E1EE8);

INCLUDE_ASM("asm/levels", func_L12_003E20F0);

void func_L12_003E2370(s32 a0) {
    s32 s0 = a0;
    s32 s1 = *(u32 *)(s0 + 0x68);
    s32 v = *(s32 *)(*(u32 *)(s1 + 0x294));

    if (v != -1) {
        s32 base = D_L12_001B2E9C;
        u8 *p = (u8 *)((u32)base + (v << 8));

        if (p != 0) {
            u8 t = *(u8 *)(p + 0x20);

            if (t != 0xFE) {
                if (t != 0xFD) {
                    if (v != ((s0 - base) >> 8))
                        return;
                }
            }
        }
    }
    func_L12_0031E1D0(s0, 0xF, 5);
    {
        s32 i = (s0 - D_L12_001B2E9C) >> 8;

        *(u16 *)(s1 + 0x29C) = 0x12C;
        *(s32 *)(*(u32 *)(s1 + 0x294)) = i;
    }
}

void func_L12_003E2418(s32 a0) {
    s32 *slot = *(s32 **)(*(u32 *)(a0 + 0x68) + 0x294);

    if (*slot == ((a0 - D_L12_001B2E9C) >> 8))
        *slot = -1;
}

INCLUDE_ASM("asm/levels", func_L12_003E2460);

INCLUDE_ASM("asm/levels", func_L12_003E3A88);

INCLUDE_ASM("asm/levels", func_L12_00400900);

void func_L12_00400B28(s32 a0, s32 a1, s32 a2, s32 a3) {
    s16 *p = D_L12_001FB960[a2];

    if (p != 0) {
        do {
            func_L12_00400900(*(u16 *)p & 0x7FFF, a1, a3);
        } while (*p++ >= 0);
    }
}

INCLUDE_ASM("asm/levels", func_L12_00407DE8);

INCLUDE_ASM("asm/levels", func_L12_00408050);

INCLUDE_ASM("asm/levels", func_L12_004081F8);

INCLUDE_ASM("asm/levels", func_L12_00408368);

INCLUDE_ASM("asm/levels", func_L12_00419E40);

INCLUDE_ASM("asm/levels", func_L12_0041A2B8);

INCLUDE_ASM("asm/levels", func_L12_0041A460);

void func_L12_0041A530(s32 a0) {
    func_L12_00322AF8(a0, *(u32 *)(a0 + 0x68) + 0x50);
    if (*(u16 *)(a0 + 0x96) <= 0xFFFE)
        *(u16 *)(a0 + 0x96) = *(u16 *)(a0 + 0x96) + 1;
}

/* Near miss: nonmatching/levels/func_L12_0041A578.c - the scan and the returns are right,
   the index lives in $a2 here against retail's in-place `addiu $a1,$a1,1`, and this cc1
   folds the loop back into the float test (`bc1fl`) where retail keeps `bc1t` + `b`. */
INCLUDE_ASM("asm/levels", func_L12_0041A578);


void func_L12_0041A730(s32 a0) {
    s16 *p = D_L12_001FB960[a0];
    s32 base;

    if (p == 0)
        return;
    base = D_L12_001B2E9C;
    do {
        u32 v = (*(u16 *)p & 0x7FFF) << 8;

        if (*(s16 *)(base + v + 0xAA) == 0xBB8) {
            u8 *r = (u8 *)*(u32 *)(base + v + 0x68);

            *(u32 *)(r + 0x798) = *(u32 *)(r + 0x798) + 1;
        }
    } while (*p++ >= 0);
}

INCLUDE_ASM("asm/levels", func_L12_00424078);

INCLUDE_ASM("asm/levels", func_L12_00426C08);

INCLUDE_ASM("asm/levels", func_L12_004277F0);


void func_L12_00427CB0(s32 a0) {
    s32 s1 = a0;
    s32 s0 = *(u32 *)(s1 + 0x68);

    func_L12_002F15C8(s0 + 0x330);
    func_L12_002F15C8(s0 + 0x332);
    if (*(u16 *)(s1 + 0x96) <= 0xFFFE)
        *(u16 *)(s1 + 0x96) = *(u16 *)(s1 + 0x96) + 1;
    s0 = s0 + 0x280;
    func_L12_00326948(s1, s0, 0, 0.03f, 0.3f);
    func_L12_003296A8(s1, s0, 0, 1, 1.8f);
}

void func_L12_00427D50(s32 a0) {
    s32 base;
    s16 *p;
    s16 *q;

    if (*(u8 *)(a0 + 0x21) == 0xFF)
        return;
    q = D_L12_001FB960[*(u8 *)(a0 + 0x21)];
    if (q == 0)
        return;
    p = q;
    base = D_L12_001B2E9C;
    do {
        u32 v = (*(u16 *)p & 0x7FFF) << 8;

        if (*(s8 *)(base + v + 0x20) >= 0) {
            if (*(s16 *)(base + v + 0xAA) == 0x11EB)
                *(u16 *)(*(u32 *)(base + v + 0x68) + 0x336) = 1;
        }
    } while (*p++ >= 0);
}

INCLUDE_ASM("asm/levels", func_L12_00427DD0);

INCLUDE_ASM("asm/levels", func_L12_00428040);

INCLUDE_ASM("asm/levels", func_L12_004281D8);

INCLUDE_ASM("asm/levels", func_L12_00429A50);

INCLUDE_ASM("asm/levels", func_L12_00429D60);

INCLUDE_ASM("asm/levels", func_L12_00429ED8);
