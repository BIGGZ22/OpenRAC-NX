#include "include_asm.h"

extern int func_00351E58(unsigned char *);

typedef int s32;

typedef unsigned char u8;

typedef unsigned int u32;

int func_00351EE8(int *owner)
{
    int result;
    long status;
    status = func_00351E58((unsigned char *)owner);
    if (status == 0)
        result = *owner + owner[2] * 0xd0000;
    else
        result = 0;
    return result;
}

s32 func_00351F28(u8 *a0) { return *(u32 *)(a0 + 0xc) < 1; }
