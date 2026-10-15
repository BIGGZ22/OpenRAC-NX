#include "include_asm.h"
#include "common.h"

extern void func_3F7CD8(s32);
extern void func_3F7EF8(s32);
extern void func_3F80A0(s32);
extern void func_3F8A98(s32);
extern void func_3F8C08(s32);

extern float func_309900(float, float);
extern float func_308EE8(float);
extern float func_308F00(float);
extern float func_3099E8(float, float);
extern float func_308860(float);
extern void func_34B1D0(s32);

INCLUDE_ASM("asm/levels", func_L18_002E68B0);

INCLUDE_ASM("asm/levels", func_L18_003170F8);

INCLUDE_ASM("asm/levels", func_L18_0033D768);

INCLUDE_ASM("asm/levels", func_L18_0035B6B8);

void func_L18_0035B838(char *a0)
{
    char *s1 = a0;
    char *s0 = s1 + 0x20;
    float t;

    *(u8 *)(s1 + 8) = *(u8 *)(s1 + 8) + *(u8 *)(s0 + 0x14);
    *(float *)(s0 + 8) = func_309900(*(float *)(s0 + 8), *(float *)(s0 + 0xC));
    *(float *)(s1 + 0xC) = *(float *)(s1 + 0xC) + *(float *)(s0 + 0x10);
    *(float *)(s1 + 0x10) = *(float *)(*(char **)(s1 + 0x20)) +
                            *(float *)(s0 + 4) * func_308EE8(*(float *)(s0 + 8));
    *(float *)(s1 + 0x18) = *(float *)(*(char **)(s1 + 0x20) + 8) +
                            *(float *)(s0 + 4) * func_308F00(*(float *)(s0 + 8));
    t = func_3099E8(*(float *)(s0 + 8), *(float *)(*(char **)(s1 + 0x20) + 0x10));
    if (t < func_308860(*(float *)(s0 + 0xC) + *(float *)(s0 + 0xC)))
        func_34B1D0(s1);
}

INCLUDE_ASM("asm/levels", func_L18_003A0C28);

INCLUDE_ASM("asm/levels", func_L18_003B12C0);

void func_L18_003F7C70(s32 a0)
{
    u8 v = *(u8 *)(a0 + 0x20);

    if (v == 0) {
        func_3F7CD8(a0);
        return;
    }
    if (v == 1)
        return;
    func_3F7EF8(a0);
    func_3F80A0(a0);
    func_3F8A98(a0);
    func_3F8C08(a0);
}

INCLUDE_ASM("asm/levels", func_L18_003F7CD8);

INCLUDE_ASM("asm/levels", func_L18_003F7EF8);
