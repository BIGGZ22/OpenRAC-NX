/* 0 of 27 places match - last line: 00_aranos_tutorial: 136 bytes, expected 144.
   Complete: the count from the table's +0xC, the descending loop, the two
   per-entry arguments and the one-based return all reproduce; only the register
   assignment differs (this cc1: table data in $v0, the +0x20 base in $a2, the
   count in $v1; retail: data and the base share $v1, the count lives in $v0),
   which also folds the +0x20 base into the load offset and drops the padding
   `nop` the retail loop carries.  The table is materialised absolutely in the
   retail bytes, so it carries nosda. */

#include "include_asm.h"

typedef int s32;
typedef unsigned char u8;

extern u8 D_L00_001D9340[] __attribute__((nosda));

extern int func_00283D80(void *, void *, void *);

/* walks the table's entries downwards and returns the one-based index of the
   first entry the shared test accepts, zero when none does */
s32 func_L00_00303388(u8 *object)
{
    u8 *data = D_L00_001D9340;
    u8 *table = data + 0x20;
    s32 i = *(s32 *)(data + 0xC) - 1;

    for (; i >= 0; i--) {
        if (func_00283D80(object, table + (i << 4), table + (i << 4) + 0x20) != 0)
            return i + 1;
    }
    return 0;
}
