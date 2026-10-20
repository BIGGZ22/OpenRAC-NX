#include "include_asm.h"

typedef unsigned int u32;
typedef short s16;

extern int D_L02_gp10B0 __attribute__((sda));
extern unsigned int D_001B00A4 __attribute__((nosda));

extern int func_002F1BE0(void *);
extern s16 func_0031E7B0(int, unsigned int);
extern void func_00366B30(int, int, void *);

void func_L02_003CB090(char *a0) {
    char *s0 = *(char **)(a0 + 0x68);

    if (func_002F1BE0(s0 + 0x3B4) != 0) {
        *(s16 *)(s0 + 0x3B4) = func_0031E7B0(D_L02_gp10B0, D_001B00A4);
        func_00366B30(1, 0, a0);
    }
}
