#include "include_asm.h"

extern void func_00335E38(char *p, float x, float y, float zero, float zero2);

extern float D_L04_001AE540 __attribute__((sda));

extern float D_L04_001AE548 __attribute__((sda));

extern float D_L04_001AE550 __attribute__((sda));

extern float D_L04_001AE558 __attribute__((sda));

extern float D_L04_001AE544;

extern float D_L04_001AE54C;

extern float D_L04_001AE554;

extern float D_L04_001AE55C;

struct pos54dfe946 {
    float x;
    float y;
};

void func_00344848(char *self)
{
    float zero = 0.0f;

    func_00335E38(self + 808, D_L04_001AE540 + (*(struct pos54dfe946 **)(self + 1072))->x,
            D_L04_001AE544 + (*(struct pos54dfe946 **)(self + 1072))->y, zero, zero);
    func_00335E38(self + 880, D_L04_001AE548 + (*(struct pos54dfe946 **)(self + 1072))->x,
            D_L04_001AE54C + (*(struct pos54dfe946 **)(self + 1072))->y, zero, zero);
    func_00335E38(self + 952, D_L04_001AE550 + (*(struct pos54dfe946 **)(self + 1072))->x,
            D_L04_001AE554 + (*(struct pos54dfe946 **)(self + 1072))->y, zero, zero);
    func_00335E38(self + 736, D_L04_001AE558 + (*(struct pos54dfe946 **)(self + 1072))->x,
            D_L04_001AE55C + (*(struct pos54dfe946 **)(self + 1072))->y, zero, zero);
}

INCLUDE_ASM("asm/game", func_00344938);

INCLUDE_ASM("asm/game", func_00344F98);

INCLUDE_ASM("asm/game", func_00345088);

INCLUDE_ASM("asm/game", func_00345178);

INCLUDE_ASM("asm/game", func_00345268);
