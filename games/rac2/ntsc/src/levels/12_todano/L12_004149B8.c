#include "include_asm.h"

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;

/* A pointer table of level-12 data, reached absolutely (out of the $gp window). */
extern s32 *D_L12_001FEAC0[];

extern f32 func_002F2950(f32, f32);

extern void func_002A0F48(void *);
extern void func_00282F20(f32 *, void *, f32);

INCLUDE_ASM("asm/levels", func_L12_0041BDF8);

extern void func_L12_0041BED8(u8 *);

/* Returns 1 as soon as one of the two bound sub-objects carries a live state word. */
s32 func_L12_0041BE60(u8 *a0)
{
    u8 *a1 = a0;
    u8 *s0;
    u8 **p;
    s32 i;

    if (a1 == 0)
        return 0;
    if (*(s16 *)(a1 + 0xAA) != 0xFAE)
        return 0;
    s0 = *(u8 **)(a1 + 0x68);
    func_L12_0041BED8(a1);
    p = (u8 **)(s0 + 8);
    for (i = 0; i < 2; i++, p++) {
        if (*p != 0) {
            if (*(s32 *)(*p + 0x98) != 0)
                return 1;
        }
    }
    return 0;
}

/* --- func_L12_0041A698 (52 B) ------------------------------------------------------ */

typedef struct {
    char pad0[0x110];
    s32 f110;             /* 0x110 */
    char pad114[0x10c];
    u32 f220;             /* 0x220 */
} Tab698;

s32 func_L12_0041A698(u8 *self)
{
    Tab698 *n = *(Tab698 **)(self + 0x68);

    return (n->f220 ^ (u32)(*D_L12_001FEAC0[n->f110] - 1)) < 1;
}

/* --- func_L12_00416390 (88 B) ------------------------------------------------------ */

typedef struct {
    char pad0[0x20];
    u8 f20;               /* 0x20 */
    char pad21[0x0f];
    u8 f30;               /* 0x30 */
    u16 f32;              /* 0x32 */
    char pad34[0xc4];
    f32 fF8;              /* 0xF8 */
} Obj390;

f32 func_L12_00416390(Obj390 *self)
{
    if (self->f20 == 0) {
        self->f20 = 1;
        self->f30 = 0x20;
        self->f32 = 0x20;
    }
    return self->fF8 = func_002F2950(self->fF8, 0.017453294f);
}

/* --- func_L12_0041BDA8 (68 B) ------------------------------------------------------ */

void func_L12_0041BDA8(u8 *a0)
{
    s32 i;
    u8 **p;

    if (a0 == 0)
        return;
    if (*(s16 *)(a0 + 0xAA) != 0xFAE)
        return;
    p = (u8 **)(*(u8 **)(a0 + 0x68) + 8);
    for (i = 1; i >= 0; i--, p++) {
        if (*p != 0)
            *(u32 *)(*p + 0x98) = 0;
    }
}

/* func_L12_0041BDF8 (104 B) and func_L12_0041BE60 (120 B): best attempt reaches every
   instruction but one — the retail keeps the incoming pointer in $a1 (a copy of $a0 at
   entry) and mine folds it onto $a0.  Left as markers; both attempts are in the wave's
   drafts/ directory. */

/* ---------------------------------------------------------------------------------- */

extern void func_326F10(void *, void *, s32, s32);

extern void func_L09_003F7788(void *, void *);

/* the two bound sub-objects of the moby this driver walks */
typedef struct {
    char pad00[0x20];
    u8 f20;                 /* 0x20 */
    char pad21[0x31 - 0x21];
    u8 f31;                 /* 0x31 */
    char pad32[0x34 - 0x32];
    u16 f34;                /* 0x34 */
    char pad36[0xAA - 0x36];
    s16 fAA;                /* 0xAA */
} Sub114B;

typedef struct {
    char pad00[0x2F0];
    Sub114B *f2F0[2];       /* 0x2F0 */
    Sub114B *f2F8;          /* 0x2F8 */
} Moby49B8;

typedef struct {
    char pad00[0x31];
    u8 f31;                 /* 0x31 */
    char pad32[0x68 - 0x32];
    Moby49B8 *f68;          /* 0x68 */
} Ent49B8;

/* Walks the moby's two sub-objects, re-arms each one's parent link and releases it. */
void func_L12_004149B8(Ent49B8 *a0)
{
    Moby49B8 *moby = a0->f68;
    Sub114B *head = moby->f2F8;
    s32 i;

    if (head == 0)
        return;
    if (head->fAA != 0x114B)
        return;
    if (head->f20 == 0xFE)
        return;
    if (head->f20 == 0xFD)
        return;
    for (i = 0; i < 2; i++) {
        if (moby->f2F0[i] != 0) {
            func_326F10(moby->f2F8, moby->f2F0[i], i, (i ^ 1) ? 0 : 6);
            if (a0->f31 != 0) {
                moby->f2F0[i]->f34 = moby->f2F0[i]->f34 & 0xFFFE;
                moby->f2F0[i]->f31 = 1;
            } else {
                moby->f2F0[i]->f34 = moby->f2F0[i]->f34 | 1;
                moby->f2F0[i]->f31 = 0;
            }
            func_L09_003F7788(a0, moby->f2F0[i]);
        }
    }
}

INCLUDE_ASM("asm/levels", func_L12_00414AB8);

INCLUDE_ASM("asm/levels", func_L12_00414C60);

INCLUDE_ASM("asm/levels", func_L12_00414EB8);

INCLUDE_ASM("asm/levels", func_L12_00415128);

INCLUDE_ASM("asm/levels", func_L12_004163E8);

INCLUDE_ASM("asm/levels", func_L12_00417690);

INCLUDE_ASM("asm/levels", func_L12_00417CA8);

INCLUDE_ASM("asm/levels", func_L12_00418418);

INCLUDE_ASM("asm/levels", func_L12_00418D20);

INCLUDE_ASM("asm/levels", func_L12_004190A8);

INCLUDE_ASM("asm/levels", func_L12_00419D68);

/* func_L12_0041A5F8 (160 B): best attempt is 156 bytes with the node pointer in $v1
   where the retail has it in $a3, and the 128-bit copy using a displacement where the
   retail pre-computes both bases.  Attempt kept in the wave's drafts/ directory. */

INCLUDE_ASM("asm/levels", func_L12_0041A5F8);

INCLUDE_ASM("asm/levels", func_L12_0041A790);

INCLUDE_ASM("asm/levels", func_L12_0041AEF0);

INCLUDE_ASM("asm/levels", func_L12_0041B2A8);

/* --- func_L12_0041BED8 (176 B) ----------------------------------------------------- */

void func_L12_0041BED8(u8 *self)
{
    u8 *s1 = *(u8 **)(self + 0x68);
    u8 **s0 = (u8 **)(s1 + 8);
    s32 i;

    for (i = 1; i >= 0; i--, s0++) {
        if (*s0 != 0) {
            f32 v = (*(f32 *)((u8 *)s0 + 0x2C) + *(f32 *)(s1 + 0x40) * 0.5f)
                    * 0.16666667f;
            f32 t;

            func_00282F20((f32 *)(*s0 + 0xD0), *s0 + 0xD0, v);
            func_002A0F48(*s0);
            t = *(f32 *)(*(u8 **)(*s0 + 0x24) + 0x3C);
            *(f32 *)(*s0 + 0xC) = t + t;
        }
    }
}

INCLUDE_ASM("asm/levels", func_L12_0041BF88);

INCLUDE_ASM("asm/levels", func_L12_0041CF10);

INCLUDE_ASM("asm/levels", func_L12_0041D3B0);

INCLUDE_ASM("asm/levels", func_L12_0041D808);

INCLUDE_ASM("asm/levels", func_L12_0041DDA8);
