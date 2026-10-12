#include "include_asm.h"

extern int ref_L00_002FBF60_00;

extern int ref_L00_002FC008_00;

extern int ref_L00_002FC018_00;

extern int ref_L00_002FC020_00;

extern int ref_L00_002FC028_00;

void func_L00_002FBF60(void)
{
    ref_L00_002FBF60_00 = 1;
}

void func_L00_002FC008(void)
{
    ref_L00_002FC008_00 = 1;
}

int func_L00_002FC018(void)
{
    return ref_L00_002FC018_00;
}

void func_L00_002FC020(void)
{
    ref_L00_002FC020_00 = 0;
}

void func_L00_002FC028(void)
{
    ref_L00_002FC028_00 = 0;
}

INCLUDE_ASM("asm/levels", func_L00_002FC058);

INCLUDE_ASM("asm/levels", func_L00_002FC460);

INCLUDE_ASM("asm/levels", func_L00_002FC580);

INCLUDE_ASM("asm/levels", func_L00_002FC588);

INCLUDE_ASM("asm/levels", func_L00_002FC5A8);

INCLUDE_ASM("asm/levels", func_L00_002FC5C8);

INCLUDE_ASM("asm/levels", func_L00_002FC5E8);

INCLUDE_ASM("asm/levels", func_L00_002FC798);
