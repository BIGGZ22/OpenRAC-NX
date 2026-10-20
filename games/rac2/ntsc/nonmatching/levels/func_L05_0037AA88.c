/* 0 of 4 places match - best candidate is 68 bytes against 64.
   The chain walk is right; the block layout of the two exits is not. Retail keeps `bltz $a0,ret` at
   the top and puts the `return -1` AFTER the loop (`jr $ra; addiu $v0,$zero,-1` at the end), with
   the product in $a1 (`mult $a1,$a0,$t0`, 40 in $t0) and the two base addresses derived from each
   other (a2 = 0x2A1804, a3 = a2 - 4).
   Every C shape tried makes this cc1 either invert the branch (bgez first, the -1 arm inline: 68
   bytes) or duplicate the loop test (84-92 bytes): `while (x == 0)`, `for (;;)` with an early
   return, the `if (index < 0) return -1;` / `if (index >= 0) { ... }` pair, hoisting the offset into
   a pointer variable, and indexing the array instead of the byte offset.
   Data for a retry: the slot record is 40 bytes, fields at 0x00 (state), 0x04 (next index),
   0x10 (limit), 0x14 (value); func_L05_0037AB18 and func_L05_0037AB88 - both matched, in
   src/levels/shared/L05_003008C0.c - use the same record through `L05Slot40 D_002A1800[]`. */

int func_L05_0037AA88(int index)
{
    int off;

    if (index >= 0) {
        off = index * 40;
        for (;;) {
            if (*(int *)((char *)D_002A1800 + off) != 0)
                return index;
            index = *(int *)((char *)D_002A1800 + off + 4);
        }
    }
    return -1;
}
