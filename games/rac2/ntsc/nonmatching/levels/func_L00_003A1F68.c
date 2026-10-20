/* Near miss: the structure is recovered, one instruction pair is scheduled differently.
   Retail at 0x003A1F68, level 00 (172 bytes, 27 places) copies a 16-byte element between one of
   two operands and the destination when the blend factor is exactly 0.0 or 1.0, and otherwise
   runs the two bound helpers and the bound into the destination:

     void f(Vec16 *dst, Vec16 *first, Vec16 *second, float t)
     {
         u128 local;                                  (mode(TI) reaches lq/sq, COMPILER-NOTES)
         if (t == 0.0f)      *(u128 *)dst = *(u128 *)first;
         else if (t == 1.0f) *(u128 *)dst = *(u128 *)second;
         else {
             func_00282CF0_9((char *)&local, (char *)second, (char *)first);
             func_00282D30_3((char *)&local, (char *)&local, t);
             func_00282CC0_15((char *)dst, (char *)first, (char *)&local);
         }
     }

   probes "as measured": `u128` via __attribute__((mode(TI))) is what produces the lq/sq (a plain
   16-byte struct assignment instead emits four ldl/ldr + sdl/sdr pairs and comes out 212 bytes);
   every other instruction, the prologue, the two float compares and the three calls agree.

   The remaining gap is the delay slot of the first copy, 8 bytes (164 against 172):
     retail  lq $v0,0x0($s0) | sq $v0,0x0($s1) | b <epilogue> | ld $s0,0x10($sp)
     ours    lq $v0,0x0($s0) | b <epilogue>    | sq $v0,0x0($s1)
   retail emits the sq before the branch and lets the filler duplicate the epilogue's first load;
   ours postpones the sq into the slot. Tried and did not move it: an if/else-if chain instead of
   the two early returns, and a `char local[16]` spell of the buffer. A scheduling difference of
   the filler, not of the C: reached via a flag or a different copy spelling. */
