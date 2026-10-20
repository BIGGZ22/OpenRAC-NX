/* func_L00_002BA138 (228 bytes, 27 places, shared) -- Not a C row: MMI.

   Two 128-bit pairs this cc1 never emits from C (see func_L16_00411600.c and the
   other rows of the same family). ASM_FUNC candidate; the shape around them is a
   prologue with $s0-$s4 saves, a `lui/lw` pair on the level's resident-state block
   and a call before the lq/sq region, so it belongs with the ASM_FUNC batch of the
   file rather than in this table's C work. */
