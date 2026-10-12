#include "include_asm.h"

extern void func_00335E38_1(char *p, float x, float y, float zero, float zero2);

extern float D_L04_001AE1E0 __attribute__((sda));

extern float D_L04_001AE1E8 __attribute__((sda));

extern float D_L04_001AE1F0 __attribute__((sda));

extern float D_L04_001AE1F8 __attribute__((sda));

extern float D_L04_001AE1E4;

extern float D_L04_001AE1EC;

extern float D_L04_001AE1F4;

extern float D_L04_001AE1FC;

struct pos {
    float x;
    float y;
};

void func_0033EC98(char *self)
{
    float zero = 0.0f;

    func_00335E38_1(self + 1256, D_L04_001AE1F8 + (*(struct pos **)(self + 680))->x,
            D_L04_001AE1FC + (*(struct pos **)(self + 680))->y, zero, zero);
    func_00335E38_1(self + 1040, D_L04_001AE1E0 + (*(struct pos **)(self + 680))->x,
            D_L04_001AE1E4 + (*(struct pos **)(self + 680))->y, zero, zero);
    func_00335E38_1(self + 1112, D_L04_001AE1E8 + (*(struct pos **)(self + 680))->x,
            D_L04_001AE1EC + (*(struct pos **)(self + 680))->y, zero, zero);
    func_00335E38_1(self + 1184, D_L04_001AE1F0 + (*(struct pos **)(self + 680))->x,
            D_L04_001AE1F4 + (*(struct pos **)(self + 680))->y, zero, zero);
}

INCLUDE_ASM("asm/game", func_0033ED88);
