#include "include_asm.h"

extern int ref_L00_0030CAA0_00 __attribute__((sda));
extern int ref_L00_0030CAA0_01 __attribute__((sda));
extern int ref_L00_0030CAA0_02 __attribute__((sda));

extern int D_001A79F0;

int func_0030CAA0(void) {
    return D_001A79F0 == 20 && ref_L00_0030CAA0_00 == 2
        ? ref_L00_0030CAA0_01 : ref_L00_0030CAA0_02;
}
