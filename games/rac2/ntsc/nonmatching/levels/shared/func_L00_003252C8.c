#include "common.h"

extern s32 D_L00_001A8FF0;
extern void func_00282C98(void *);
extern void func_002AFFD8();

void func_L00_003252C8(a0, a1)
void *a0;
void *a1;
{
    if (D_L00_001A8FF0 == 0)
        func_00282C98(a1);
    else
        func_002AFFD8(a0, a1, 1);
}
