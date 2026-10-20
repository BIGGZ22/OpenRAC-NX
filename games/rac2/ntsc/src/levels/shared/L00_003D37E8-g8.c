#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L00_003DC2D0);

extern int D_L00_001B043C __attribute__((sda));

extern int D_L00_001B2B44 __attribute__((sda));

int func_L00_003DE260(void)
{
    char *p = (char *)D_L00_001B043C;

    if (p == 0)
        return 0;
    return (unsigned int)((unsigned char)p[0x20] - 5) < 3;
}

int func_L00_003DF108(void)
{
    if (D_L00_001B043C == 0)
        return 0;
    return D_L00_001B2B44;
}

INCLUDE_ASM("asm/levels", func_L00_003DF6B0);

extern int D_L00_001B19C8[] __attribute__((sda));

extern char *D_L00_001B1EA8[] __attribute__((sda));

extern float D_L00_001B0430 __attribute__((sda));

extern float func_002EE2B0(char *a, char *b);

extern void func_00381F80(char *a, char *b);

/* walks the object list once the shared flag is set, stopping at either special state and
   otherwise acting on the entries whose handle reports type 0x13 */
void func_L00_003DF030(char *arg)
{
    char *o;

    if ((D_L00_001B19C8[0] & 1) == 0)
        return;

    for (o = D_L00_001B1EA8[0]; o != 0; o = *(char **)(o + 40)) {
        unsigned char state = *(unsigned char *)(o + 32);

        if (state == 0xFE)
            return;
        if (state == 0xFD)
            return;
        if (o == 0 || *(int *)(o + 36) == 0 || *(short *)(*(int *)(o + 36) + 70) != 0x13)
            continue;
        if (func_002EE2B0(o + 16, arg + 16) < D_L00_001B0430)
            func_00381F80(o, arg);
    }
}
