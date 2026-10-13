#include "include_asm.h"

extern float D_001AE180 __attribute__((sda));
extern float D_001AE17C;
extern float D_001AE178 __attribute__((sda));
extern void func_00335E38(void *, float, float, float, float);
void func_003423C8(char *self)
{
    int index = *(int *)(self + 0x16C);
    float *v = *(float **)(self + 0x180);
    func_00335E38(self + 0xE4, D_001AE178 + v[0],
            (D_001AE17C + D_001AE180 * (float)index) + v[1],
            0.0f, 0.0f);
}
