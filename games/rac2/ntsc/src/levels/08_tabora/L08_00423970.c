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

/* Walks the 0x68 context's two counters: one stamps the record's byte 0x20D and the other
   gives the modulus the first is folded by, or zero when it is not positive. */
void func_L08_00424ED0(char *a0) {
    char *q;
    s32 d;
    if (*(u8 *)(a0 + 0x21) == 0xFF) {
        return;
    }
    q = *(char **)(a0 + 0x68);
    if (*(u8 *)(q + 0x20D) == *(s32 *)(*(char **)(q + 0x1F8))) {
        *(s32 *)(*(char **)(q + 0x1F8)) = *(u8 *)(q + 0x20D) + 1;
    }
    d = *(s32 *)(*(char **)(q + 0x1FC));
    if (d > 0) {
        *(s32 *)(*(char **)(q + 0x1F8)) = *(s32 *)(*(char **)(q + 0x1F8)) % d;
    } else {
        *(s32 *)(*(char **)(q + 0x1F8)) = 0;
    }
}

INCLUDE_ASM("asm/levels", func_L08_00424F40);

INCLUDE_ASM("asm/levels", func_L08_00424FF8);

/* Not C: the success arm is the single word 0x4600A028 (COP1 fmt=S funct=0x28,
   fd=$f0 fs=$f0 ft=$f20). See nonmatching/levels/func_L08_00425328.c - this cc1 has no
   max.s pattern, so the ternary and the if-assignment both come out eight bytes larger. */
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

/* Near miss: nonmatching/levels/func_L08_004254D0.c - the table load and the shift land in
   $v0/$v1 the other way round from retail, and `a0 = 0` is not sunk into the beqz slot. */
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

extern void func_L08_00425398(char *);
extern int func_002B0ED8(char *, char *, int);
extern void func_00282CF0(void *, void *, void *);
extern void func_00282F20(void *, void *, float);
extern char D_00189EA0[];

/* The record's whole per-frame pass: the sibling step, the aim vector copied out of the
   sub-object, the shared sprite pass, then the dispatch the shared tag call returns. */
void func_L08_00425F28(char *a0) {
    char *q = *(char **)(a0 + 0x68);
    int r;

    func_L08_00425398(a0);
    *(f32 *)(a0 + 0x2C) = *(f32 *)(*(char **)(a0 + 0x24) + 0x24);
    func_L08_00425EB0(a0);
    r = func_002B0ED8(a0, q + 0x50, 0);
    if (r == 0) {
        return;
    }
    if (r == 1) {
        float v[4];

        func_00282CF0(v, a0 + 0x10, D_00189EA0);
        func_00282F20(q + 0x1A0, v, 0.16666667f);
    } else {
        if (r == 4) {
            *(unsigned char *)(q + 0xC6) = 0;
            *(s32 *)(q + 0xCC) = -1;
        }
        func_L08_00424ED0(a0);
    }
    func_L08_00425448(a0);
}

INCLUDE_ASM("asm/levels", func_L08_00425FE8);

INCLUDE_ASM("asm/levels", func_L08_004262C0);

INCLUDE_ASM("asm/levels", func_L08_00426730);

INCLUDE_ASM("asm/levels", func_L08_004290E8);

INCLUDE_ASM("asm/levels", func_L08_00429238);

INCLUDE_ASM("asm/levels", func_L08_00429350);

extern s32 D_L08_001B39A0[];
extern s32 D_L08_001B39E0[];
extern s32 D_L08_001B39F0[];
extern s32 D_L08_001B3A30[];
extern void func_00291450(s32 a0);

/* Near miss: nonmatching/levels/func_L08_00429408.c - the address materialisation order
   in the prologue differs (0x1B39E0 / 0x1B39A0 / 0x1B3A30 are grouped differently). */
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
