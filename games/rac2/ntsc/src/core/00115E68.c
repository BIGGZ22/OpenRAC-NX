#include "common.h"

/* $ra is saved in a TImode slot (`sq $ra,0($sp)`), so this unit is built with
   the -mastra-quad-saves profile. The 64-bit result comes back truncated to
   the 32-bit return type, which is retail's dsll32/dsra32 pair. */
extern long func_001175F0(int arg0, int arg1, int arg2);

int func_00115E68(int arg0) {
    return (int)func_001175F0(arg0, 0, 10);
}
