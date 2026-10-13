#include "include_asm.h"

extern int ref_L00_002F3458_00;

void func_00287E60(void)
{
    ref_L00_002F3458_00 = 3;
}

extern short D_001BAD04 __attribute__((nosda));

int func_00287E70(void) {
    return D_001BAD04;
}

void func_00287E80(void) {
    D_001BAD04 = 0;
}

extern int D_001A8A64 __attribute__((sda));

int func_00287E90(void) {
    return D_001A8A64;
}
