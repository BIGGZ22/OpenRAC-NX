/* func_L22_003FD140, 168 bytes, 1 placement.  NOT ATTEMPTED - the catalogue entry starts
   16 bytes before the body, so no C can cover the recorded span.
   The first four words are `lwc1 $f0,0x44($v0)` (a $v0 the body never sets, i.e. a neighbour's
   tail) followed by three nops; the body's own `addiu $sp,$sp,-0x70` opens at 0x003FD150 and
   the epilogue `jr $ra` / `addiu $sp,$sp,0x70` closes at 0x003FD1E4-0x003FD1E7.  The real
   entry is 0x003FD150 with a size of 152 bytes - data for the maintainers, as the wave-6 note
   asks for this family (the same defect was measured in levels 23 and 30).
   Decoded body, for whoever fixes the entry: zero the 16-byte stack slot at sp+0 with
   `sq $zero,0x0($sp)`, store 0.018873f (0x3C9AE466) at sp+8, func_2F42E0(sp+0x10, sp, ...)
   builds the second slot, s0 = a0 + 0xC0 and s2 = *(a0+0x68) + 0x10, then
   func_2F4628(s0, s0, sp+0x10), func_32A1C8(a0 + 0x10, s2, 0) and
   func_32A5F8(s0, s2, 0) - the two 0x10 offsets are the 16-byte transforms. */
