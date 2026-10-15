#include "common.h"

typedef int TI128 __attribute__((mode(TI)));

extern void func_L16_00410CC0(u8 *, u8 *);

extern void func_2E74B8(void *, void *, void *);

extern float func_2E7590(void *, void *);

extern s32 func_313EA0(s32);

extern void func_30AD48(void *, s32, void *);

extern float func_313F38(float, float);

extern void func_41F1F0(void *, void *, void *, s32, float, float);

extern void func_00282DD8(void *, void *, void *);

extern void func_002ACE78(void *, void *, float);

extern void func_002A7750(void *, s32, s32);

extern void func_0029FCC0(void *, s32, void *);

extern void func_0029FC20(void *, s32, void *);

extern void func_002A0F48(void *);

extern void func_00283678(void *, void *);

extern void func_002A7470(void *, s32, s32, void *);

extern void func_00282CF0(void *, void *, void *);

extern void func_00282F20(void *, void *, float);

extern float func_00282E48(void *, void *);

extern float func_00282E80(void *, void *);

extern void func_L07_00321D68(void *, void *);

extern void func_L02_00327350(s32, s32);

extern void func_L01_00323310(s32, s32);

extern void func_L00_003294F0(void *, s32);

extern void func_002DF3F8(void *, void *);

extern void func_0029F338(void *);

extern void func_0029B3E0(s32, s32);

extern void func_L16_00412C58(void *, s32, s32);

extern void func_L16_004117A8(u8 *);

extern void func_L16_00411F80(u8 *, s32);

extern void func_L16_0041ECF8(void *, float);

extern void func_002AD7B8(s32);

extern void func_00282948(void *);

extern void func_L16_004050D8(u8 *);

extern void func_0029F978(u8 *, s32, void *);

extern void func_00283898(void *, s32, float);

extern float func_00283BE0(float, float);

extern void func_002A8C00(void *, u8 *, s32, void *, float);

extern void func_00282CC0(void *, void *, void *);

extern float func_00282C48(float);

extern void func_002A8E20(s32, void *);

INCLUDE_ASM("asm/levels", func_L16_00403900);

/* func_L16_00404A60 is in the sibling file L16_00403900-g8.c: its body
   reaches the level's small-data floats through $gp. */

INCLUDE_ASM("asm/levels", func_L16_00404E40);

INCLUDE_ASM("asm/levels", func_L16_00404F50);

INCLUDE_ASM("asm/levels", func_L16_004050D8);

INCLUDE_ASM("asm/levels", func_L16_00405488);

INCLUDE_ASM("asm/levels", func_L16_004056A8);

INCLUDE_ASM("asm/levels", func_L16_0040C840);

INCLUDE_ASM("asm/levels", func_L16_0040CD88);

INCLUDE_ASM("asm/levels", func_L16_0040CF98);

INCLUDE_ASM("asm/levels", func_L16_0040D188);

INCLUDE_ASM("asm/levels", func_L16_0040D258);

INCLUDE_ASM("asm/levels", func_L16_0040D4E8);

INCLUDE_ASM("asm/levels", func_L16_0040DAE0);

INCLUDE_ASM("asm/levels", func_L16_0040DC70);

INCLUDE_ASM("asm/levels", func_L16_0040ECC0);

INCLUDE_ASM("asm/levels", func_L16_00410618);

/* func_L16_00410B88 is defined in L16_00403900-g8.c (this unit has no $gp access for it). */

/* Copies the measured 32-bit field pair of the handle reached at 0x68 onto the second
   pair after the helper has prepared the object's second half. */
void func_L16_00410C80(u8 *arg)
{
    u8 *s0;

    s0 = *(u8 **)(arg + 0x68);
    func_L16_00410CC0(arg, arg + 0x10);
    *(s32 *)(s0 + 0x220) = *(s32 *)(s0 + 0x1F4);
    *(s32 *)(s0 + 0x224) = *(s32 *)(s0 + 0x1F8);
}

INCLUDE_ASM("asm/levels", func_L16_00410CC0);

INCLUDE_ASM("asm/levels", func_L16_00410E40);

INCLUDE_ASM("asm/levels", func_L16_00410FA8);

INCLUDE_ASM("asm/levels", func_L16_004110D0);

INCLUDE_ASM("asm/levels", func_L16_00411368);

INCLUDE_ASM("asm/levels", func_L16_00411480);

INCLUDE_ASM("asm/levels", func_L16_00411600);

/* Routes one transition request to the handle itself or to its first child, depending on
   the state word and on which of the two animation slots is current. */
void func_L16_004116C8(u8 *arg0, s32 arg1, void *arg2, float f)
{
    u8 *s2;

    s2 = *(u8 **)(arg0 + 0x68);
    if (*(s32 *)(s2 + 0x26C) == 0) {
        if (*(u8 *)(arg0 + 0x43) != arg1)
            func_002A7470(arg0, arg1, (s32)f, arg2);
    } else {
        if (*(u8 *)(arg0 + 0x43) != 0x16)
            func_002A7470(arg0, 0x16, (s32)f, arg2);
        if (arg1 == 0xA)
            arg1 = 0x16;
    }
    if (arg1 < 0x11)
        func_002A7470(*(u8 **)(s2 + 0x1F0), arg1, (s32)f, arg2);
    else
        func_002A7470(*(u8 **)(s2 + 0x1F0), 0, (s32)f, arg2);
}

INCLUDE_ASM("asm/levels", func_L16_004117A8);

INCLUDE_ASM("asm/levels", func_L16_00411900);

INCLUDE_ASM("asm/levels", func_L16_00411B40);

INCLUDE_ASM("asm/levels", func_L16_00411D00);

/* Builds a 16-byte query record from the game speed and the handle's byte field and
   hands it with a prepared buffer to the generic path. */
INCLUDE_ASM("asm/levels", func_L16_00411ED0);

INCLUDE_ASM("asm/levels", func_L16_00411F80);

INCLUDE_ASM("asm/levels", func_L16_00412560);

/* func_L16_00412AF0 is defined in L16_00403900-g8.c (this unit has no $gp access for it). */

INCLUDE_ASM("asm/levels", func_L16_00412BA8);

INCLUDE_ASM("asm/levels", func_L16_00412C58);

INCLUDE_ASM("asm/levels", func_L16_00412D48);

INCLUDE_ASM("asm/levels", func_L16_00412E70);

INCLUDE_ASM("asm/levels", func_L16_00412F48);

INCLUDE_ASM("asm/levels", func_L16_004130B0);

INCLUDE_ASM("asm/levels", func_L16_004133B0);

/* Prepares a 16-byte record from the handle and reports one when the queried value
   does not exceed zero. */
s32 func_L16_00413638(u8 *arg)
{
    s32 buf[4];
    u8 *s0;

    s0 = *(u8 **)(arg + 0x68);
    func_2E74B8(buf, s0 + 0x10, arg + 0x10);
    if (func_2E7590(s0, buf) <= 0.0f)
        return 1;
    return 0;
}

INCLUDE_ASM("asm/levels", func_L16_00413700);

INCLUDE_ASM("asm/levels", func_L16_00413D10);

INCLUDE_ASM("asm/levels", func_L16_00413EC8);

