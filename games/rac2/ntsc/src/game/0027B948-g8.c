#include "include_asm.h"

extern void func_002FC560(unsigned int reg, unsigned long value);

extern int D_001A72E0;

extern short D_001A6480[];

extern void func_0027DE50(int a, int b, int c, int d, unsigned long e);

/* The argument is a structure: a word at +4 and a double word at +8. */
void func_0027DCE8(char *p)
{
    if (*(unsigned long long *)(p + 8) != 0) {
        func_002FC560(0x42, *(unsigned long long *)(p + 8) & 0xFF000000FFULL);
    }
    if (*(int *)(p + 4) & 0xFF000000) {
        func_002FC560(0x4E, (((long long)(D_001A72E0 >> 13) | 0x1000000) | (1LL << 32)));
        func_0027DE50(0, D_001A6480[0xA9], 0, D_001A6480[0xA8], *(unsigned int *)(p + 4));
        func_002FC560(0x4E, (D_001A72E0 >> 13) | 0x1000000);
    }
    if (*(unsigned long long *)(p + 8) != 0) {
        func_002FC560(0x42, 0x8000000044ULL);
    }
}
