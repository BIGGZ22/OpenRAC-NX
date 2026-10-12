#include "include_asm.h"

extern int ref_L25_00438E98_00;

extern int ref_L05_00447F10_00;

int func_L05_00446E28(void)
{
    return ref_L25_00438E98_00;
}

int func_L05_00447F10(void)
{
    return ref_L05_00447F10_00;
}

INCLUDE_ASM("asm/levels", func_L05_00450238);

INCLUDE_ASM("asm/levels", func_L05_00462858);
