#include "include_asm.h"

/* Both accumulator words live in small data, so this file is built with -G8.
   They differ from program to program, so they carry no level. */
extern int ref_L00_002B62A0_00 __attribute__((sda));
extern float ref_L00_002B62A0_01 __attribute__((sda));

/* Counts one tick and folds the hardware timer, converted from unsigned, into
   the small-data float accumulator. */
void func_002B62A0(void)
{
    unsigned int ticks = *(volatile unsigned int *)0x10000000;

    ref_L00_002B62A0_00 = ref_L00_002B62A0_00 + 1;
    ref_L00_002B62A0_01 = ref_L00_002B62A0_01 + (float)ticks;
}
