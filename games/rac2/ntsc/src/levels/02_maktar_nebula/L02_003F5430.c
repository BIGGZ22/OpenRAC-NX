#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;
typedef int s32;

extern char *D_L02_001B3AF8 __attribute__((sda));
extern s32 func_002F3220(char *, float);

void func_L02_003F5430(char *a0) {
    char *s0 = *(char **)(a0 + 0x68);
    s32 i;
    s16 *s2;
    *(u32 *)(s0 + 0x84) = 0;
    for (i = 0; i < *(s32 *)(s0 + 0x80); i++) {
        char *q;
        s2 = (s16 *)(s0 + 0x40 + i * 2);
        q = D_L02_001B3AF8 + *s2 * 32;
        if (func_002F3220(q, *(float *)(q + 0x10)) != -1)
            *(u32 *)(s0 + 0x84) |= 1 << i;
    }
}

INCLUDE_ASM("asm/levels", func_L02_003F54D8);

INCLUDE_ASM("asm/levels", func_L02_003F5710);

extern int D_L02_002A5520[];

INCLUDE_ASM("asm/levels", func_L02_003F9AF0);

extern int D_L02_002A5560[];

INCLUDE_ASM("asm/levels", func_L02_003F9B70);

INCLUDE_ASM("asm/levels", func_L02_003F9C10);

extern int func_00282948(void *);
extern void func_L00_002E6EF8(char *, char *);
extern void func_L00_0030CF60(void);
extern char D_L02_004004D8[];

void func_L02_003FAF28(char *a0) {
    char *s0 = a0;
    char *obj = *(char **)(s0 + 0x68);
    u16 timer = *(u16 *)(s0 + 0x96);

    if (timer <= 0xFFFE)
        *(u16 *)(s0 + 0x96) = timer + 1;
    func_00282948(obj + 0xE4);
    if (*(u8 *)(s0 + 0x20) > 1)
        func_L00_002E6EF8(D_L02_004004D8, s0);
    func_L00_0030CF60();
}

extern void func_002A7750(char *, s32, s32);

void func_L02_003FAF90(char *a0) {
    char *s0 = *(char **)(a0 + 0x68);
    u32 *p;
    s32 i;
    *(s16 *)(s0 + 0x120) = -1;
    *(s16 *)(s0 + 0x122) = -1;
    *(u16 *)(s0 + 0x118) = 0;
    p = (u32 *)(s0 + 0xBC);
    for (i = 5; i >= 0; i--) {
        *p = 0;
        p--;
    }
    *(u32 *)(s0 + 0xD8) = 0;
    *(u32 *)(s0 + 0xD0) = 0;
    func_002A7750(a0, 1, -1);
    *(u32 *)(s0 + 0xC0) = 0;
}

INCLUDE_ASM("asm/levels", func_L02_003FB000);

INCLUDE_ASM("asm/levels", func_L02_003FB120);

INCLUDE_ASM("asm/levels", func_L02_003FB4D8);

INCLUDE_ASM("asm/levels", func_L02_003FB5A0);

INCLUDE_ASM("asm/levels", func_L02_003FB6C0);

INCLUDE_ASM("asm/levels", func_L02_003FB7E0);

extern s32 func_003D77F8(char *);
extern void func_003D78A8(char *);

void func_L02_003FBA58(char *a0) {
    char **p;
    char *s1 = *(char **)(a0 + 0x68);
    s32 i;
    for (i = 0; i <= *(s32 *)(s1 + 0xD8); i++) {
        p = (char **)(s1 + 0xA8 + i * 4);
        if (*p != 0) {
            if (func_003D77F8(*p) != 0)
                func_003D78A8(*p);
        }
    }
}

extern s32 func_L02_003D7830(char *);

/* True when every occupied slot of the list at +0xA8 has content; then the list is cleared. */
void func_L02_003FBAD0(char *a0) {
    char *s1 = *(char **)(a0 + 0x68);
    char **p = (char **)(s1 + 0xA8);
    s32 i;
    s32 j;
    s32 ok = 1;

    for (i = 0; i <= *(s32 *)(s1 + 0xD8); i++) {
        if (*p != 0) {
            if (func_L02_003D7830(*p) == 0) {
                ok = 0;
                break;
            }
        }
        p++;
    }
    if (ok) {
        for (j = 0; j <= *(s32 *)(s1 + 0xD8); j++)
            *(s32 *)(s1 + 0xA8 + j * 4) = 0;
        *(s32 *)(s1 + 0xD8) = 0;
    }
}

INCLUDE_ASM("asm/levels", func_L02_003FBB90);

INCLUDE_ASM("asm/levels", func_L02_003FBCF8);

extern s32 func_L02_00400650(char *, s32);
extern char D_L02_001BA3C0[];

void func_L02_003FC020(char *a0, char *a1) {
    char *a3 = *(char **)(a0 + 0x68);
    char *g = D_L02_001BA3C0;
    s16 t;
    if (*(u32 *)(g + 0x284) == *(u32 *)(a3 + 0xD0))
        *(u32 *)(g + 0x284) = 0;
    if (*(u32 *)(g + 0x284) == 0) {
        *(u32 *)(g + 0x284) = (u32)a1;
        *(u32 *)(g + 0x288) = 0;
    }
    *(u32 *)(a3 + 0xD0) = (u32)a1;
    t = *(s16 *)(a1 + 0xAA);
    if (t == 0x9F8)
        func_L02_00400650(a0, 0xF);
    else if (t == 0xA44)
        func_L02_00400650(a0, 0x15);
}

extern char *D_L02_001B368C __attribute__((sda));

char *func_L02_003FC098(char *a0) {
    char *a1 = *(char **)(a0 + 0x68);
    return D_L02_001B368C + *(u32 *)(a1 + 0x88) * 128 + 0x30;
}

s32 func_L02_003FC0B8(char *a0) {
    char *v1 = *(char **)(a0 + 0x68);
    return *(s32 *)(v1 + 0x84);
}

INCLUDE_ASM("asm/levels", func_L02_003FC0D0);

INCLUDE_ASM("asm/levels", func_L02_003FC1B0);

INCLUDE_ASM("asm/levels", func_L02_003FC268);

INCLUDE_ASM("asm/levels", func_L02_003FC518);

INCLUDE_ASM("asm/levels", func_L02_003FC7C8);

INCLUDE_ASM("asm/levels", func_L02_003FC9C0);

INCLUDE_ASM("asm/levels", func_L02_003FCA80);

/* func_L02_003FCBF0 is defined in L02_003F5430-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L02_003FCCA0);

extern void func_0029FCC0(char *, s32, char *);
extern float func_00282E80(char *, char *);
extern char D_00189EA0[];

s32 func_L02_003FD000(char *a0) {
    char buf[16];
    s32 r = 0;
    func_0029FCC0(a0, 0, buf);
    if (*(u8 *)(a0 + 0x31) != 0) {
        if (func_00282E80(buf, D_00189EA0) > 25.0f)
            r = 1;
    }
    return r;
}

INCLUDE_ASM("asm/levels", func_L02_003FD070);

INCLUDE_ASM("asm/levels", func_L02_003FD228);

INCLUDE_ASM("asm/levels", func_L02_003FD7C8);

INCLUDE_ASM("asm/levels", func_L02_003FD8B8);

INCLUDE_ASM("asm/levels", func_L02_003FDDE8);

extern int D_L02_002A5600[];

INCLUDE_ASM("asm/levels", func_L02_003FE1B0);

INCLUDE_ASM("asm/levels", func_L02_003FE250);

INCLUDE_ASM("asm/levels", func_L02_003FE8B8);

INCLUDE_ASM("asm/levels", func_L02_003FF6E8);

INCLUDE_ASM("asm/levels", func_L02_003FF9A0);

INCLUDE_ASM("asm/levels", func_L02_003FFDB0);

INCLUDE_ASM("asm/levels", func_L02_004002E0);

INCLUDE_ASM("asm/levels", func_L02_004004D0);

extern char D_001A63A8[];

/* Timer for one of the thirty-nine messages, refused while one is running. */
s32 func_L02_00400650(char *a0, s32 a1) {
    char *p = *(char **)(a0 + 0x68);
    s32 v;

    if ((u32)a1 >= 0x27)
        return 0;
    if (a1 < 0x14) {
        s32 i = a1;
        char *q = p + 0x130;
        v = *(s32 *)(q + i * 4);
    } else {
        s32 i = a1 - 0x14;
        char *q = p + 0x180;
        v = *(s32 *)(q + i * 4);
    }
    {
        char *g = D_001A63A8;

        if (*(s32 *)(g + 0x68) != 0)
            goto fail;
        if (*(s32 *)(g + 0x24) != -1)
            goto fail;
        *(s32 *)(g + 0x28) = 0;
        *(s32 *)(g + 0x24) = (v & 0x3FFF) + 0x7D0;
    }
    {
        char *q = p + 0x1D0;
        *(u16 *)(q + a1 * 2) = 1;
    }
    return 1;
fail:
    return 0;
}

INCLUDE_ASM("asm/levels", func_L02_00400720);

INCLUDE_ASM("asm/levels", func_L02_00400948);

INCLUDE_ASM("asm/levels", func_L02_00400B10);

extern int D_0017A80;

/* True when the field at +0xAA is 0xB59, the global at D_0017A80 is 1 and bit 3 of the
   byte at +0x20 is clear. */
s32 func_L02_00400CC8(char *a0) {
    s32 r = 0;
    if (*(s16 *)(a0 + 0xAA) == 0xB59 && D_0017A80 == 1)
        r = (*(u8 *)(a0 + 0x20) ^ 8) ? 0 : D_0017A80;
    return r;
}

INCLUDE_ASM("asm/levels", func_L02_00401298);

INCLUDE_ASM("asm/levels", func_L02_00401638);

INCLUDE_ASM("asm/levels", func_L02_00401BF8);

INCLUDE_ASM("asm/levels", func_L02_00401D38);

INCLUDE_ASM("asm/levels", func_L02_00409D78);
