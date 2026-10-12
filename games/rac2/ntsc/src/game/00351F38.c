#include "include_asm.h"

extern int func_00351F28(void *a0);

typedef int s32;

typedef unsigned char u8;

/* Returns zero when a helper reports non-zero, otherwise combines three fields of the pointed-to block (a difference plus a third term, taken modulo that third term) scaled by a constant and added to the first field. */
int func_00351F38(volatile int *a0)
{
    int d, n;

    if (func_00351F28((void *)a0) != 0)
        return 0;
    d = a0[2] - a0[3];
    n = a0[4];
    d = d + n;
    return a0[1] + (d % n) * 0x138c0;
}

void func_00351FA0(u8 *a0) {
 if(*(volatile s32 *)(a0+0xc)>0) *(volatile s32 *)(a0+0xc)=*(volatile s32 *)(a0+0xc)-1;
}
