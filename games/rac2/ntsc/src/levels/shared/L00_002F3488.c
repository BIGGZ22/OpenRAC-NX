#include "include_asm.h"

extern int ref_L00_002F3488_00;

extern int ref_L00_002F3490_00;

extern int ref_L00_002F34A0_00;

int func_L00_002F3488(void)
{
    return ref_L00_002F3488_00;
}

void func_L00_002F3490(void)
{
    ref_L00_002F3490_00 = 1;
}

void func_L00_002F34A0(void)
{
    ref_L00_002F34A0_00 = 0;
}

INCLUDE_ASM("asm/levels", func_L00_002F3DA8);
