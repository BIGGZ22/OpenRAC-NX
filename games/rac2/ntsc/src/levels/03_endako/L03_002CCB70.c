#include "common.h"

INCLUDE_ASM("asm/levels", func_L03_002CCB70);

INCLUDE_ASM("asm/levels", func_L03_002D3498);

INCLUDE_ASM("asm/levels", func_L03_002FDE30);

extern u8 *D_L03_001B2918;

s32 func_L03_003112F8(void)
{
    u8 *p = D_L03_001B2918;
    u8 *q;
    s32 n = 0;

    if (p == 0)
        return 1;
    q = p;
    while (*(s16 *)q != 0) {
        if (*(s16 *)(q + 0x24) == 1) {
            if ((*(u16 *)(q + 0x10) & 2) == 0)
                n++;
        }
        q += 0x28;
    }
    return n;
}

INCLUDE_ASM("asm/levels", func_L03_00329ED0);

extern void *func_L00_0032C638(char *a0);
extern float func_00283248(float a, float b);
extern float func_00283B98(float a, float b);
extern int func_00282978(short *p);
extern int func_002A7820(int a0, int a1);
/* The callee's parameters are listed in the order the retail code sets the
   registers up in: int, int, float, int, float, float. */
extern int func_L02_00331440(char *a0, char *a1, float f12, char *a2, float f13, float f14);

/* Aim the record at the shape the caller found. */
int func_L03_0032CFD8(char *self, float *target, char *third, float amount, float f13, float f14)
{
    char *data = (char *)func_L00_0032C638(self);
    float angle;

    if (data == 0)
        return 8;
    angle = func_00283248(target[0] - *(float *)(self + 0x10),
                         target[1] - *(float *)(self + 0x14));
    angle = func_00283B98(angle, amount);
    if (*(float *)(data + 0xDC) != 0.0f) {
        angle = func_00283B98(angle, *(float *)(data + 0xDC));
        if (func_00282978((short *)(data + 0xE0))) {
            *(float *)(data + 0xDC) = -*(float *)(data + 0xDC);
            *(short *)(data + 0xE0) = func_002A7820(*(short *)(data + 0xE2), *(short *)(data + 0xE4));
        }
    }
    return func_L02_00331440(self, target, angle, third, f13, f14);
}

INCLUDE_ASM("asm/levels", func_L03_00383FE8);

INCLUDE_ASM("asm/levels", func_L03_00384340);

INCLUDE_ASM("asm/levels", func_L03_003844D8);

INCLUDE_ASM("asm/levels", func_L03_0038AA20);

INCLUDE_ASM("asm/levels", func_L03_00393838);

INCLUDE_ASM("asm/levels", func_L03_00393C50);
