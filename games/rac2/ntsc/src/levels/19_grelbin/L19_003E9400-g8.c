#include "include_asm.h"

extern float D_L19_001B1D28 __attribute__((sda));

extern void func_2EB590();

extern void func_3238E0(void *, void *, int, int, float);

void func_L19_0041C2B8(char *a0)
{
    char *p = a0;
    int obj = *(int *)(p + 0x68);

    func_2EB590(obj + 0x230);
    func_2EB590(obj + 0x234);
    if (*(unsigned short *)(p + 0x96) < 0xFFFF) {
        *(unsigned short *)(p + 0x96) += 1;
    }
    func_3238E0(p, obj + 0x2A0, 1, 1, D_L19_001B1D28);
}
