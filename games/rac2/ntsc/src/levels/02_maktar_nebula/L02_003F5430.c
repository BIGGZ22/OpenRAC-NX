#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;
typedef int s32;

INCLUDE_ASM("asm/levels", func_L02_003F5430);

INCLUDE_ASM("asm/levels", func_L02_003F54D8);

INCLUDE_ASM("asm/levels", func_L02_003F5710);

extern int D_L02_002A5520[];

INCLUDE_ASM("asm/levels", func_L02_003F9AF0);

extern int D_L02_002A5560[];

INCLUDE_ASM("asm/levels", func_L02_003F9B70);

INCLUDE_ASM("asm/levels", func_L02_003F9C10);

INCLUDE_ASM("asm/levels", func_L02_003FAF28);

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

INCLUDE_ASM("asm/levels", func_L02_003FBA58);

INCLUDE_ASM("asm/levels", func_L02_003FBAD0);

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

INCLUDE_ASM("asm/levels", func_L02_003FC098);

INCLUDE_ASM("asm/levels", func_L02_003FC0B8);

INCLUDE_ASM("asm/levels", func_L02_003FC0D0);

INCLUDE_ASM("asm/levels", func_L02_003FC1B0);

INCLUDE_ASM("asm/levels", func_L02_003FC268);

INCLUDE_ASM("asm/levels", func_L02_003FC518);

INCLUDE_ASM("asm/levels", func_L02_003FC7C8);

INCLUDE_ASM("asm/levels", func_L02_003FC9C0);

INCLUDE_ASM("asm/levels", func_L02_003FCA80);

INCLUDE_ASM("asm/levels", func_L02_003FCBF0);

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

INCLUDE_ASM("asm/levels", func_L02_00400650);

INCLUDE_ASM("asm/levels", func_L02_00400720);

INCLUDE_ASM("asm/levels", func_L02_00400948);

INCLUDE_ASM("asm/levels", func_L02_00400B10);

INCLUDE_ASM("asm/levels", func_L02_00400CC8);

INCLUDE_ASM("asm/levels", func_L02_00401298);

INCLUDE_ASM("asm/levels", func_L02_00401638);

INCLUDE_ASM("asm/levels", func_L02_00401BF8);

INCLUDE_ASM("asm/levels", func_L02_00401D38);

INCLUDE_ASM("asm/levels", func_L02_00409D78);
