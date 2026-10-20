/* func_L00_002B9BB0, 168 bytes, 27 placements.  ABANDON - register-allocation wall.
   Takes or refreshes the record the slot names: when the slot entry is not the one the record
   at 0x188860 (stride 0x70) still points at through its +0x88 word, or the record's +0x74
   flag byte is clear, the helper runs with the caller's handle, mode 4 and the wanted value,
   and its result replaces the slot entry. The companion array at +0x2450 always records the
   wanted value. The condition is the negation of `record->f88 == wanted && record->f74 != 0`,
   which is what the two branches (`bnel` then `bnez`) encode.
   Candidate is 160 bytes (two short) and the structure is exact, but retail keeps the resident
   high half in `$s3`, builds `base+0x242C` in `$v0` and COMPLETES the base a second time for
   the +0x2450 store (`addiu $v0,$s3,-0x61E0`), where this cc1 reuses the earlier
   `base+0x242C` register and adds the index; the index/table registers also swap (`$v0`/`$a0`
   against `$v1`/`$s3`). Naming the symbol directly at the last store did not change it.
   Unblocking: see func_L00_002A74A8 - the same high-half-live wall. */

#include "include_asm.h"

extern char D_00189E20_2[];
extern char D_00188860[];
extern int func_35F3A8_r(void *a0, int a1, int a2) __asm__("func_35F3A8");

void func_L00_002B9BB0(int index, int wanted, void *handle)
{
    char *base = D_00189E20_2;
    int *slot = (int *)(base + 0x242C) + index;
    char *record = D_00188860 + *slot * 0x70;

    if (*(int *)(record + 0x88) != wanted || *(unsigned char *)(record + 0x74) == 0)
        *slot = func_35F3A8_r(handle, 4, wanted);
    *(int *)(D_00189E20_2 + 0x2450 + index * 4) = wanted;
}
