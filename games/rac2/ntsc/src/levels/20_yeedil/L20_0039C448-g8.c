#include "include_asm.h"

/* 0x1AEFF0 + 0x39F0 : this symbol is written through $gp (gas emits the
 * gp-relative form because the store sits in the bc1f delay slot), so the
 * file has to be compiled with -G8 and the symbol marked sda. */
extern float D_L20_001B29E0 __attribute__((sda));

/* 0x1AEFF0 + 0x39F0 again, under a second name: gcc must not see the two
 * stores as one dead store, and gas expands this one to lui $at / swc1. */
extern float D_L20_001B29E0_1;

void func_L20_004095D0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    float v = *(float *)(state + 0xC) * 1.2f;

    D_L20_001B29E0 = v;
    if (v > 1.0f) {
        D_L20_001B29E0_1 = 1.0f;
    }
}
