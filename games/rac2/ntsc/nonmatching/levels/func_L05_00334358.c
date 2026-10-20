/* Not a C row: the extent opens on `addiu $sp,$sp,0x60; nop; addiu $sp,$sp,0xA0` - two stack
   restores and their padding, in level 05 (5 places). No function starts by adding to the stack
   pointer. The row is the gap between func_002A8C28 (0x334210, 0x148, ends exactly here) and the
   real entry func_002A8E20 at 0x334368.
   Maintainer data: no C spelling; 12 dead bytes to absorb into the neighbour. */
