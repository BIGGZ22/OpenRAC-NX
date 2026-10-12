#include "include_asm.h"

/* The two small-data words below are reached through $gp in ordinary flow, not
   in a delay slot, so this unit has to be the -G8 sibling of L05_00428350.c. */
extern int D_L05_001B1B20 __attribute__((sda));

extern int D_L05_001B1B24 __attribute__((sda));

/* Reports 1 while the second word is set; otherwise whether the first word,
   minus two, is below two as an unsigned value. */
int func_L05_00442860(void)
{
    if (D_L05_001B1B24 != 0)
        return 1;
    return (unsigned int)(D_L05_001B1B20 - 2) < 2;
}
