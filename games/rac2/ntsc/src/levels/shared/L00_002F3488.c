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

extern unsigned char D_00139568[];

extern char D_L00_00262620[];

/* One byte of the resident table, scaled by 0xE0 into the level's own table. */
int func_L00_002F3DA8(int a0)
{
    return D_00139568[a0] * 0xE0 + (int)D_L00_00262620;
}
