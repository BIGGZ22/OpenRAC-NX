/* snd_SendCurrentBatch of 989snd.  Identity: rac2's func_00133220 is
   rac1-decomp src/core/989snd.c func_0012EC30, a delta of +0x45F0 for this
   object, which puts 0x00133108 on rac1's func_0012EB18 -- the body rac1
   keeps as `INCLUDE_ASM ... /* snd_SendCurrentBatch *​/` (line 634 of that
   file), so there is no rac1 C to carry over either.

   Shape: two parallel tables selected by the global at $gp-0x7B30
   (0x001A74C0, flipped 0/1 at the end), func_001322B8 on the pair, then a
   poll of sceSifCheckStatRpc (func_0011D810) on the client at 0x001A7040
   and one sceSifCallRpc (func_0011D620) with rpc number 0x4D, mode 1 and
   the two 0x1000-byte buffers.  func_00115D48(0x001A7578) and
   func_0011AEA0(0) sit on the non-ready arm.

   Not attempted as C in this run.  Two things have to be in place first:
   the unit needs a `-g8.c` sibling with `__attribute__((sda))` on
   0x001A74A0/0x001A74A8/0x001A74B8/0x001A74C0/0x001A74F8 to get the
   `$gp`-relative forms retail has, and the half of the sceSifCheckStatRpc
   poll that reaches func_00115D48 does not fall out of any obvious
   `while`/`do` wording yet -- the ready arm branches to the send path and
   the not-ready arm back into the poll, so the loop has to be written to
   rotate the way retail's does. */
