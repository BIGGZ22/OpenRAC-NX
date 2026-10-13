#include "common.h"

extern void func_L16_00410CC0(u8 *, u8 *);

extern void func_2E74B8(void *, void *, void *);

extern float func_2E7590(void *, void *);

extern s32 func_313EA0(s32);

extern void func_30AD48(void *, s32, void *);

extern float func_313F38(float, float);

extern void func_41F1F0(void *, void *, void *, s32, float, float);

INCLUDE_ASM("asm/levels", func_L16_00403900);

INCLUDE_ASM("asm/levels", func_L16_00404A60);

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

INCLUDE_ASM("asm/levels", func_L16_00410B88);

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

INCLUDE_ASM("asm/levels", func_L16_004116C8);

INCLUDE_ASM("asm/levels", func_L16_004117A8);

INCLUDE_ASM("asm/levels", func_L16_00411900);

INCLUDE_ASM("asm/levels", func_L16_00411B40);

INCLUDE_ASM("asm/levels", func_L16_00411D00);

/* Builds a 16-byte query record from the game speed and the handle's byte field and
   hands it with a prepared buffer to the generic path. */
void func_L16_00411ED0(u8 *arg)
{
    u8 *s0 = arg;
    u8 *s1 = *(u8 **)(arg + 0x68);
    s32 buf[4];
    s32 rec[4];
    float t;

    func_30AD48(s0, func_313EA0(2), buf);
    qzero(rec);
    t = func_313F38(5.0f, 10.0f);
    *(float *)((u8 *)rec + 8) = -(t * (1.0f / 60.0f));
    func_41F1F0(s0, buf, rec, 1, 1.0f / 180.0f, (float)*(u8 *)(s1 + 0x36));
}

INCLUDE_ASM("asm/levels", func_L16_00411F80);

INCLUDE_ASM("asm/levels", func_L16_00412560);

INCLUDE_ASM("asm/levels", func_L16_00412AF0);

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

INCLUDE_ASM("asm/levels", func_L16_00413690);

INCLUDE_ASM("asm/levels", func_L16_00413700);

INCLUDE_ASM("asm/levels", func_L16_00413D10);

INCLUDE_ASM("asm/levels", func_L16_00413EC8);


