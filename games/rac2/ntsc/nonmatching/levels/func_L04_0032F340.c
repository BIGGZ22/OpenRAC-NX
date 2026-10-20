/* func_L04_0032F340 (128 bytes, 5 places) -- NEAR MISS, not integrated: COP2/VU0.
   The whole row is VU0 code: eight `ldc2` of two 0x40-byte blocks into vector registers
   $vf2-$vf9, a run of macro-mode words (`0x4BE111BF`, `0x4BE13088`, `0x4BE119BF`,
   `0x4BE138C8`, `0x4BE121BF`, `0x4BE14108`, `0x4BE129BF`, `0x4BE14948`) that do the work,
   four `sdc2` stores, and a 23-iteration counter (`addiu $t2,$zero,0x17` / `addi $t2,$t2,-1`
   / `bnez`). Nothing here is C: the macro-mode words are inline-assembly or intrinsics in
   the original source, and the R5900 pipeline behaviour they depend on is not reachable
   from the Plain C rule. Measured: not attempted beyond the decode. The `0x4BE1xxxx` family
   is the same one already recorded as "not C" for the other levels. */
