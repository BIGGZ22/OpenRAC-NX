/* Retained wave8/w10 draft, rechecked 2026-10-15.
   Compiler rebuilt from master bb4c620; -O2 -G8 -ffunction-sections.
   48 bytes versus 52; 0 of 3 placements match. Address scheduling and the filled return delay slot differ.
   No matching credit or behavioral validation. Original C body retained;
   missing declarations supplied and calls named from the current catalogue.
   Reproduce: copy this file to src/levels/shared/ with the same filename,
   remove its original INCLUDE_ASM marker, and run tools/check.py func_L05_00442828 --diff.
   Reopen when the compiler/source can reproduce the measured differences. */

#include "include_asm.h"

/* Belongs in L05_00428350-g8.c: both small words reach $gp in ordinary flow. */
typedef int TI128 __attribute__((mode(TI)));

extern TI128 D_L05_001B3610;

extern TI128 D_L05_001B3620;

extern int D_L05_001B1B20 __attribute__((sda));

extern int D_L05_001B1B24 __attribute__((sda));

/* Puts the two bound 16-byte records in place and latches the flag word. */
void func_L05_00442828(char *a0, char *a1, int a2)
{
    D_L05_001B1B24 = a2;
    D_L05_001B1B20 = 1;
    *(TI128 *)&D_L05_001B3610 = *(TI128 *)a1;
    *(TI128 *)&D_L05_001B3620 = *(TI128 *)a0;
}
