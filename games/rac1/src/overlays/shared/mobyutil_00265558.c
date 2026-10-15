/* Shared level code following mobyutil. */
#include "common.h"
#include "include_asm.h"

extern int D_L00_0015F674 MACRO_ADDR;
extern int D_L00_0015F670 MACRO_ADDR;
extern int D_L00_00161F04 MACRO_ADDR;
extern int func_001FFCB0(int);

int func_L00_00265558(int a) {
    if (D_L00_0015F674 == a) {
        if (D_L00_00161F04 != -1) {
            D_L00_0015F670 = 0;
            D_L00_0015F674 = 0;
            func_001FFCB0(D_L00_00161F04);
            D_L00_00161F04 = -1;
            return 1;
        }
    }
    return 0;
}

INCLUDE_ASM("asm/overlays", func_L00_002657B8);
