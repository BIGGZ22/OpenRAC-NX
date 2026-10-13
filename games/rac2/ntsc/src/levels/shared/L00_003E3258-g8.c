#include "include_asm.h"

extern int D_001A72E0;

extern int D_001A72E0_1;

extern short D_001A6480[];

extern void func_002FC560(int id, long long value);

extern void func_0027DE50(int a, int b, int c, int d, int e);

extern char D_L00_0024EFC0[];

extern int D_L00_001B2B60;

extern void func_00282A88(char *buf, int size);

void func_L00_003E55B0(char *object)
{
    char *s1;

    s1 = *(char **)(object + 0x68);
    if (*(int *)(s1 + 0x24) != 0) {
        func_002FC560(0x4E, (((long long)(D_001A72E0 >> 13) | 0x1000000) | (1LL << 32)));
        func_0027DE50(0, D_001A6480[0xA9], 0, D_001A6480[0xA8], (*(int *)(s1 + 0x24) << 24) | 0xFFFFFF);
        func_002FC560(0x4E, (D_001A72E0_1 >> 13) | 0x1000000);
    }
}

void func_L00_003E8150(void)
{
    func_00282A88(D_L00_0024EFC0, 0x800);
    D_L00_001B2B60 = 0;
}
