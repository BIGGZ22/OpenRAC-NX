/* 0 of 5 places match - last line: 04_barlow: instruction at +0x1c differs.
   The body is instruction-exact except for one register: this cc1 fetches the
   word into $v0, the register that already holds the computed entry base, where
   retail loads it into $v1 and keeps the base in $v0 for the store.  A distinct
   result local and an int-expression load both reproduce the rest word for
   word.  Unblocked by a register-allocation lever, not by another source shape.

   1 of 27 places (the level the name states) also rejects the binding: the two
   fetched words let the checker read the same symbol at two addresses. */

#include "include_asm.h"

typedef int s32;
typedef unsigned char u8;

/* resolves an entry through the object's state block and caches the pointed-to
   table slot on the object, then clears the flag at the head of its sub-object */
void func_L04_003A1578(u8 *object, s32 index, s32 *table)
{
    u8 *state = *(u8 **)(object + 0x68);
    s32 value;

    if (state[0x14] == 0)
        value = *(s32 *)(((*(s32 *)(state + 0x1C) << 4) + index) + 0x4);
    else
        value = *(s32 *)(((*(s32 *)(state + 0x1C) << 4) + index) + 0x8);

    *(s32 **)(object + 0x98) = table + value;
    *(s32 *)(*(u8 **)(object + 0x24) + 0x10) = 0;
}
