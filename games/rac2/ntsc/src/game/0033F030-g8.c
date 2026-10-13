#include "include_asm.h"

extern float D_001AE0E0 __attribute__((sda));
extern float D_001AE0DC;
extern float D_001AE0D8 __attribute__((sda));
extern void func_00335E38(void *, float, float, float, float);

void func_00341760(char *self)
{
    int index = *(int *)(self + 0x2FC);
    float *v = *(float **)(self + 0x218);
    int *base = (int *)(self + (index << 2));

    func_00335E38(self + 0x130, D_001AE0D8 + v[0],
            (D_001AE0DC + D_001AE0E0 * (float)base[0x6E]) + v[1],
            0.0f, 0.0f);
}
