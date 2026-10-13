#include "include_asm.h"

extern int D_L05_001B0F50 __attribute__((sda));

void func_L05_00417730(char *object)
{
    char *state = *(char **)(object + 0x68);

    D_L05_001B0F50 = 0;
    *(int *)(state + 0x1BC) = 0;
    *(int *)(state + 0x1B8) = 0;
}

extern void func_00282C88(void *target);

extern void func_00283098(void *a, void *b, void *c);

extern void func_00282CC0(void *a, void *b, void *c);

extern int func_L00_002FCF18(void *p, float, float, float, float, float);

extern int D_L05_001B0FA4 __attribute__((sda));

void func_L05_00418200(char *object)
{
    float local[4];
    float one = 1.0f;

    func_00282C88(local);
    local[0] = 4.0f;
    local[2] = one;
    func_00283098(local, local, object + 0xC0);
    func_00282CC0(local, local, object + 0x10);
    D_L05_001B0FA4 = func_L00_002FCF18(local, 0.0f, 0.0f, 0.0f, one, 0.0f);
}
