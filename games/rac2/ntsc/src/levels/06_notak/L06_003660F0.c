#include "include_asm.h"

extern unsigned char D_001A7A47;
extern void func_0029F978(void);
extern void func_0029F9F0(int a0, char *a1);

void func_L06_003660F0(int a0, int a1, char *a2, float v)
{
    if (D_001A7A47 != 0) {
        if (*(unsigned char *)(a2 + 1) == 0)
            func_0029F978();
        *(float *)(a2 + 0x20) = v;
        *(float *)(a2 + 0x24) = v;
        *(float *)(a2 + 0x28) = v;
    } else if (*(unsigned char *)(a2 + 1) != 0) {
        func_0029F9F0(a0, a2);
    }
}
