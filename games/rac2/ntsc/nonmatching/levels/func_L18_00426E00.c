/* 0 of 3 places match - last line: 18_damosel: 80 bytes, expected 72.
   The logic is measured (0 for a null object and for the bytes 0xFD/0xFE, 1 for
   the halfwords 0xEC0/0xED4/0xEE1) but this cc1 folds the two byte tests into a
   0xFD..0xFE range test, returns 0 through an early `jr $ra` instead of
   branching to the shared return, and keeps the result in $v1 where retail keeps
   it in $a1.  A switch spelling produced 100 bytes with `slti` chains, worse.
   Needs a shape that stops the range fold. */

#include "include_asm.h"

int func_L18_00426E00(unsigned char *object)
{
    int result = 0;

    if (object == 0)
        return result;
    if (object[0x20] == 0xFE || object[0x20] == 0xFD)
        return result;
    if (*(short *)(object + 0xAA) == 0xEC0 || *(short *)(object + 0xAA) == 0xED4 ||
        *(short *)(object + 0xAA) == 0xEE1)
        result = 1;
    return result;
}
