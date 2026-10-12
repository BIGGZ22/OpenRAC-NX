#include "common.h"

/* family c459a266b43242d6 - 352 bytes, 2 placements
   levels/19_grelbin @0x00412F70, levels/8_tabora @0x0041E630

   Fills five output floats from two pinned tables and a resident flag word,
   then clamps the three still free outputs to [-1, 1]. */

extern float D_00189E20_14[];

extern int D_00138180[];

/* 128-bit copies reach lq/sq only through TImode. */
typedef int TI128 __attribute__((mode(TI)));

/* A resident word: the same address and the same value in every program.  It is
   read as a symbol here (the `sda` declaration leaves the one- or two-instruction
   choice to the assembler) and as a literal in func_L08_003DB4A0, whose two
   conditional loads the literal address form is what reproduces. */
extern s32 D_001A79F0 __attribute__((sda));

/* shared callee, named by its catalogue row (08:003289D0, 19:00319288) */
extern void func_L00_00326048(void *, s32, s32, s32, float);

INCLUDE_ASM("asm/levels", func_L08_003D3928);

INCLUDE_ASM("asm/levels", func_L08_003D39F0);

/* One resident word, two states, and a 16-byte table entry: hands the entry to
   func_L00_00326048 with an alpha of 0x3D7A35E0. */
void func_L08_003D3EB8(u8 *a0)
{
    s32 v = *(s32 *)(a0 + 0x68);

    if (v == 0)
        return;
    func_L00_00326048(a0, v, 0x80AAAAD2, 0x80396464, 0.0610865355f);
}

INCLUDE_ASM("asm/levels", func_L08_003D4190);

INCLUDE_ASM("asm/levels", func_L08_003D4600);

INCLUDE_ASM("asm/levels", func_L08_003D4690);

/* Two values of the resident word 0x001A79F0 select a shared byte or word. */
s32 func_L08_003DB4A0(void)
{
    s32 v = *(s32 *)0x001A79F0;

    if (v == 8)
        return *(s16 *)0x001A7A1A;
    if (v == 0x13)
        return *(s32 *)0x001A7A1C;
    return 0;
}

/* The same resident word, but the 8 case falls through to a 1/2 selector. */
s32 func_L08_003DB4E8(void)
{
    s32 v = D_001A79F0;
    s32 r = 1;

    if (v == 8)
        return 1;
    if (v == 0x13)
        r = 2;
    return r;
}

INCLUDE_ASM("asm/levels", func_L08_003DB518);

INCLUDE_ASM("asm/levels", func_L08_003DB8E8);

INCLUDE_ASM("asm/levels", func_L08_003DE5E8);

INCLUDE_ASM("asm/levels", func_L08_003E5BB8);

INCLUDE_ASM("asm/levels", func_L08_003F6018);

INCLUDE_ASM("asm/levels", func_L08_003F6348);

INCLUDE_ASM("asm/levels", func_L08_003F6568);

INCLUDE_ASM("asm/levels", func_L08_003F6610);

INCLUDE_ASM("asm/levels", func_L08_0041E4F8);

/* shared native body of family c459a266b43242d6 */
void func_L08_0041E630(float *a0, float *a1, float *a2, float *a3, float *p4)
{
    float f1;
    float f2;
    int v1;

    *a0 = -D_00189E20_14[2028];
    *a2 = -D_00189E20_14[2029];
    v1 = D_00138180[104];
    f2 = 0.0f;
    if (v1 & 0x40)
        f2 = 1.0f;
    f1 = (v1 & 0x20) ? (f2 - 1.0f) : f2;
    *a1 = f1;
    *a3 = *(float *)&D_00138180[75];
    v1 = D_00138180[104];
    f1 = 0.0f;
    if (v1 & 0x1)
        f1 = 1.0f;
    if (v1 & 0x2)
        f1 = f1 - 1.0f;
    *p4 = f1;
    if (*a0 > 1.0f)
        *a0 = 1.0f;
    else if (*a0 < -1.0f)
        *a0 = -1.0f;
    if (*a1 > 1.0f)
        *a1 = 1.0f;
    else if (*a1 < -1.0f)
        *a1 = -1.0f;
    if (*p4 > 1.0f)
        *p4 = 1.0f;
    else if (*p4 < -1.0f)
        *p4 = -1.0f;
}

INCLUDE_ASM("asm/levels", func_L08_0041E790);

INCLUDE_ASM("asm/levels", func_L08_0041EA88);

INCLUDE_ASM("asm/levels", func_L08_0041EBF8);

INCLUDE_ASM("asm/levels", func_L08_004207D0);

INCLUDE_ASM("asm/levels", func_L08_0042E678);

INCLUDE_ASM("asm/levels", func_L08_0042EF00);

INCLUDE_ASM("asm/levels", func_L08_00435600);

/* Stores one 16-byte block through the pointer at 0x68 of the argument. */
void func_L08_004356A0(u8 *a0, void *a1)
{
    *(volatile TI128 *)*(u32 *)(a0 + 0x68) = *(TI128 *)a1;
}

INCLUDE_ASM("asm/levels", func_L08_004356B8);


INCLUDE_ASM("asm/levels", func_L08_00435990);

INCLUDE_ASM("asm/levels", func_L08_00435A78);
