#include "include_asm.h"

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef signed int     s32;
typedef float          f32;

/* Quadword copies and clears, the spelling that reaches lq/sq. */
typedef int TI128 __attribute__((mode(TI)));

extern void func_00282948(void *a0);

extern void func_00282978(void *a0);

extern void func_0029F338(void *a0);

extern u32 D_L01_001BF918[];

extern u32 g_pHeroMoby[];

extern s32 func_3120A8(void *a0, u32 a1, u32 a2, u32 a3, u32 a4, u32 a5,
                      u32 a6, f32 f12, f32 f13);

extern s32 func_355780(u32 a0, u32 a1, void *a2);

extern s32 func_2E45B8(f32 a0);

extern void func_3377C8(u32 a0);

extern void func_310060(u8 *a0, s32 a1, s32 a2);

extern void func_310418(u8 *a0, s32 a1, s32 a2);

extern u8 *D_L01_001B255C;

extern u32 D_L01_001FFE80[];

INCLUDE_ASM("asm/levels", func_L01_003CAFE0);

INCLUDE_ASM("asm/levels", func_L01_003CB540);

INCLUDE_ASM("asm/levels", func_L01_003CB668);

INCLUDE_ASM("asm/levels", func_L01_003CBCD8);

/* Frees the object at 0xD8 of the object at 0x68 and clears its 0xDC flag. */
void func_L01_003CBFA8(u8 *a0)
{
    u8 *p = *(u8 **)(a0 + 0x68);

    if (a0[0x21] != 0xFF && *(u8 *)(p + 0xDC) != 0)
        func_00282948(*(void **)(p + 0xD8));
    *(u8 *)(p + 0xDC) = 0;
}

INCLUDE_ASM("asm/levels", func_L01_003CBFF0);

/* Releases the sub-objects at 0xC0, 0xBC, 0xC4 and 0x26, first bumping the counter at 0x96. */
void func_L01_003CC210(u8 *a0)
{
    u8 *p = a0;
    u8 *q = *(u8 **)(p + 0x68);

    if (*(s32 *)(q + 0x74) == 0)
        func_00282948(q + 0xC0);
    if (*(u16 *)(p + 0x96) < 0xFFFE)
        *(u16 *)(p + 0x96) = *(u16 *)(p + 0x96) + 1;
    func_00282948(q + 0xBC);
    func_00282948(q + 0xC4);
    func_00282978(q + 0x26);
}

INCLUDE_ASM("asm/levels", func_L01_003CC280);

/* Registers the object with the 0x04000009 descriptor and arms its 0x00 flag once. */
void func_L01_003CC898(u8 *a0, u32 a1, u32 a2, u8 a3, u8 *a4)
{
    u8 *s0 = a0;
    u8 *s1 = a4;

    if (func_3120A8(s0, a1, a2, 0x04000009, 0, 1, 1, (f32)a3, 1.0f) == 0)
        return;
    if (D_L01_001BF918[0] != g_pHeroMoby[0])
        return;
    if (*s1 == 0)
        return;
    func_355780(3, 0, s0);
    *s1 = 1;
}

INCLUDE_ASM("asm/levels", func_L01_003CC930);

INCLUDE_ASM("asm/levels", func_L01_003CDB40);

/* Sends the timer string and steps the 0x2E/0x28/0x2A state fields of the owner. */
INCLUDE_ASM("asm/levels", func_L01_003CDD00);

/* Releases the three sub-objects at 0xBC, 0xB4 and 0x26, first bumping the counter at 0x96. */
void func_L01_003CDE58(u8 *a0)
{
    u8 *p = *(u8 **)(a0 + 0x68);

    if (*(u16 *)(a0 + 0x96) < 0xFFFE)
        *(u16 *)(a0 + 0x96) = *(u16 *)(a0 + 0x96) + 1;
    func_00282948(p + 0xBC);
    func_00282948(p + 0xB4);
    func_00282978(p + 0x26);
}

INCLUDE_ASM("asm/levels", func_L01_003CDEA8);

INCLUDE_ASM("asm/levels", func_L01_003CE228);

INCLUDE_ASM("asm/levels", func_L01_003CE7D8);

INCLUDE_ASM("asm/levels", func_L01_003CE868);

INCLUDE_ASM("asm/levels", func_L01_003CEDA0);

INCLUDE_ASM("asm/levels", func_L01_003D1A48);

INCLUDE_ASM("asm/levels", func_L01_003D1F40);

INCLUDE_ASM("asm/levels", func_L01_003D25B8);

INCLUDE_ASM("asm/levels", func_L01_003D26C0);

INCLUDE_ASM("asm/levels", func_L01_003D2890);

/* Marks the object at 0x68 busy and notifies the owner once the 0xBC flag is clear. */
void func_L01_003D2C38(u8 *a0)
{
    u8 *p = a0;

    *(u32 *)(*(u8 **)(p + 0x68) + 0x5C) |= 1;
    if (*(u8 *)(p + 0xBC) != 0 || *(u8 *)0x001A7B26 != 0)
        func_0029F338(p);
}

INCLUDE_ASM("asm/levels", func_L01_003D2E50);

INCLUDE_ASM("asm/levels", func_L01_003D68C0);

INCLUDE_ASM("asm/levels", func_L01_003D8358);

INCLUDE_ASM("asm/levels", func_L01_003D8830);

INCLUDE_ASM("asm/levels", func_L01_003D8AD0);

INCLUDE_ASM("asm/levels", func_L01_003D8C00);

INCLUDE_ASM("asm/levels", func_L01_003D8E80);

INCLUDE_ASM("asm/levels", func_L01_003D9830);

INCLUDE_ASM("asm/levels", func_L01_003D98E0);

INCLUDE_ASM("asm/levels", func_L01_003DA300);

INCLUDE_ASM("asm/levels", func_L01_003DA3F0);
