#include "include_asm.h"

typedef struct { int a, b; } PAIR8;

extern PAIR8 D_L00_001B1458 __attribute__((nosda));

extern int D_L00_001B1460 __attribute__((sda));

extern void func_2EE0D0(void *dst);

extern int func_3411A0(void *a0, void *a1, int a2, float f12);

void func_L00_0041ACA0(char *a0, void *a1)
{
    int local[4];
    int sel[2];
    char *p = *(char **)(a0 + 0x68);
    float f;

    func_2EE0D0(local);
    *(PAIR8 *)sel = D_L00_001B1458;
    f = *(float *)(p + 0x50) * 0.025f;
    func_3411A0(a1, local, sel[D_L00_001B1460 % 2], f);
    D_L00_001B1460 = D_L00_001B1460 + 1;
}
