/* func_L04_003524D0, 68 bytes, 2 placements.  ABANDON - register-pair wall, three words.
   Returns one of thirty table floats: the count at +0x190 is clamped to the argument, the
   short at +0x18E is reduced against it and the window, and the remainder picks a 4-byte
   slot whose float at +0x2D0 is the result. The `div` carries the divide-by-zero guard
   (`beql $a3,$zero` + `break 0,7`), which is why the window is a variable, not a literal.
   Candidate is 64 bytes. After the clamp was written as `if (count >= arg)` (the `slt`+`movz`
   form retail has, against `slt`+`movn` for the `>`/`<` spellings) and the table was given its
   own pointer so the address is completed rather than kept as a high half, only the register
   PAIR is left: retail holds the table in `$a2` and the window in `$a3`, this cc1 the other
   way round. Both declaration orders (table first, window first, window last) were measured.
   Unblocking: a spelling that makes the table's pseudo take the lower register. */

#include "include_asm.h"

extern char D_00138180[];

float func_L04_003524D0(int arg)
{
    char *table = D_00138180;
    int count = *(int *)(table + 0x190);
    int base = *(short *)(table + 0x18E);
    int window = 30;

    if (count >= arg)
        count = arg;
    return *(float *)(table + ((base - count + window) % window) * 4 + 0x2D0);
}
