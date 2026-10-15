#include "include_asm.h"

extern char *D_L07_001AEC08 __attribute__((sda));

extern float func_2E5480(float a0, float a1);

extern void func_338D20(int a0, int a1, char *a2, void *a3, int a4, float f12);

extern void func_2E4B70(char *a0, void *a1);

void func_L07_00379180(float a0) {
    char *s0 = *(char **)(D_L07_001AEC08 + 0x68);
    int v;

    func_338D20(*(int *)(s0 + 0x14), 1, s0 + 0x50, &v, 0, func_2E5480(a0 + 45.0f, 1000.0f));
    func_2E4B70(s0 + 0x20, &v);
}
