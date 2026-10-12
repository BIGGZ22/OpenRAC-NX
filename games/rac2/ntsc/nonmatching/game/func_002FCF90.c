#include "include_asm.h"

extern int *D_001B22A8;

void func_002FCF90(int a0) {
    int *p = D_001B22A8;
    int *q = p;
    p += 4;
    D_001B22A8 = p;
    q[0] = (int)((unsigned int)a0 + 0x90000000u);
    q[2] = 0;
    q[3] = 0;
    q[1] = 0;
}
