#include "include_asm.h"

extern float D_L04_001AE440 __attribute__((sda));

extern float D_L04_001AE444;

extern float D_L04_001AE448 __attribute__((sda));

extern void func_00335E38_5(void *, float, float, float, float);

extern void func_00336920(void *, int, int);

extern void func_00336828_1(void *);

void func_00342668(int *p)
{
    int i;

    for (i = 0; i < p[95]; i++) {
        float *q = (float *)p[96];
        int v;

        func_00335E38_5(p + 76, q[0] + D_L04_001AE440,
                (q[1] + D_L04_001AE444) + D_L04_001AE448 * (float)i, 0.0f, 0.0f);
        if (i == 3)
            v = 0xEA9E;
        else
            v = 0xEAA6;
        p[93] = v;
        func_00336920(p + 76, v, ((int *)p[94])[i]);
        func_00336828_1(p + 76);
    }
}

INCLUDE_ASM("asm/game", func_00342758);
