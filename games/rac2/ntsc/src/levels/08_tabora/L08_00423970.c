#include "include_asm.h"

typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef float f32;

/* Level 08 data.  tools/build.py binds D_L08_XXXXXXXX at its address in level 08, and the
   `sda` declaration is what makes cc1 emit the one-instruction form the assembler can turn
   into `lw $v,offs($gp)` at a site inside a `.set nomacro` region (a compiler delay slot). */
extern s32 D_L08_001B29DC __attribute__((sda));

extern char *func_32A170(s32 a, s32 b, s32 c);
extern void func_328A38(s32 a, s32 b);

INCLUDE_ASM("asm/levels", func_L08_00423970);

INCLUDE_ASM("asm/levels", func_L08_00424ED0);

INCLUDE_ASM("asm/levels", func_L08_00424F40);

INCLUDE_ASM("asm/levels", func_L08_00424FF8);

INCLUDE_ASM("asm/levels", func_L08_00425328);

INCLUDE_ASM("asm/levels", func_L08_00425398);

void func_L08_00425448(char *a0) {
    char *s0;
    s0 = *(char **)(a0 + 0x68);
    if (*(u8 *)(a0 + 0x21) != 0xFF) {
        func_328A38(*(u8 *)(a0 + 0x21), 1);
    }
    if (*(s32 *)(s0 + 0x224) != -1) {
        func_328A38(*(s32 *)(s0 + 0x224), 1);
    }
    if (*(s32 *)(s0 + 0x228) != -1) {
        func_328A38(*(s32 *)(s0 + 0x224), 1);
    }
    *(u8 *)(a0 + 0xBC) = 1;
}

INCLUDE_ASM("asm/levels", func_L08_004254D0);

s32 func_L08_00425518(s32 a0) {
    s32 v1;
    s32 v0;
    u8 state;
    if (a0 != -1) {
        v0 = a0 << 8;
        v1 = D_L08_001B29DC;
        v1 = v1 + v0;
        state = *(u8 *)(v1 + 0x20);
        a0 = 0;
        if (state >= 3 || v1 == 0 || state == 0xFE || state == 0xFD) {
            a0 = 1;
        }
        return a0;
    }
    return 1;
}

INCLUDE_ASM("asm/levels", func_L08_00425568);

INCLUDE_ASM("asm/levels", func_L08_00425628);

INCLUDE_ASM("asm/levels", func_L08_004257A0);

INCLUDE_ASM("asm/levels", func_L08_004258D0);

INCLUDE_ASM("asm/levels", func_L08_00425AA8);

void func_L08_00425EB0(char *a0) {
    char *p;
    char *q;
    s16 v1;
    p = func_32A170((s32)a0, -1, 1);
    if (p == 0) {
        return;
    }
    q = *(char **)(p + 0x20);
    if (q == 0) {
        return;
    }
    v1 = *(s16 *)(q + 0xAA);
    if (v1 == 0xA66 || v1 == 0xA82 || v1 == 0xCBB || v1 == 0xCF3) {
        *(f32 *)(p + 0x2C) = *(f32 *)(p + 0x2C) * 0.5f;
    }
}

INCLUDE_ASM("asm/levels", func_L08_00425F28);

INCLUDE_ASM("asm/levels", func_L08_00425FE8);

INCLUDE_ASM("asm/levels", func_L08_004262C0);

INCLUDE_ASM("asm/levels", func_L08_00426730);

INCLUDE_ASM("asm/levels", func_L08_004290E8);

INCLUDE_ASM("asm/levels", func_L08_00429238);

INCLUDE_ASM("asm/levels", func_L08_00429350);

INCLUDE_ASM("asm/levels", func_L08_00429408);

INCLUDE_ASM("asm/levels", func_L08_004294A0);

INCLUDE_ASM("asm/levels", func_L08_00429560);

INCLUDE_ASM("asm/levels", func_L08_00429EE8);

INCLUDE_ASM("asm/levels", func_L08_0042A0B0);

INCLUDE_ASM("asm/levels", func_L08_0042A980);

INCLUDE_ASM("asm/levels", func_L08_0042F440);

INCLUDE_ASM("asm/levels", func_L08_0042F660);

INCLUDE_ASM("asm/levels", func_L08_0042F938);

INCLUDE_ASM("asm/levels", func_L08_0042FAC0);

INCLUDE_ASM("asm/levels", func_L08_0042FCE0);
