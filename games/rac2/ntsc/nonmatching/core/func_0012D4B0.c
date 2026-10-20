/* Identity: rac2 = rac1 + 0x3E20 in this object (0012D100.c's func_0012D420
   is rac1 src/core/00125630.c func_00129600), so 0x0012D4B0 is rac1's
   func_00129690 -- which rac1 keeps as `INCLUDE_ASM` (line 1821 of that
   file), i.e. unmatched there too.  The bytes are identical: searching the
   rac1 boot ELF for this body finds it at 0x00129550, exactly rac1's
   0x00129690 minus the 0x140 shift the ELF has from rac1-decomp's PAL
   addresses over this region.

   It opens with `t1 = s[0x174]; t2 = s[0x150]; xori $v0,$t1,3` -- the same
   two selector fields and the same `== 3` tests as its matched neighbour
   func_00129600 in 0012D100.c -- and then runs a 692-byte dispatch with
   four arms and the same `- 1` countdown shape.  Reading func_00129600
   first is the cheapest way in.

   Not attempted as C in this run: no rac1 source, no near miss already paid
   for it, and 692 bytes of recursive arm selection is more than one
   iteration of this wave's budget. */
