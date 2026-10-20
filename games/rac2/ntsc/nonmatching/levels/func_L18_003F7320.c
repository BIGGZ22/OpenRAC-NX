/* func_L18_003F7320 (52 bytes, 3 places) -- NEAR MISS, not integrated.
   state: the `= 0` accumulator shape reaches 52 bytes but its diffs are registers and
   the early-return form; retail is one accumulator in $a2 with a single return.
   retail:
     003F7320  lhu   $v0, 0x96($a0)
     003F7324  daddu $a2, $zero, $zero      <- the accumulator, before the test
     003F7328  sltiu $v0, $v0, 0xF1
     003F732C  beqz  $v0, . + 4 + (0x6 << 2)
     003F7330  lw    $a1, 0x68($a0)         <- in the beqz delay slot
     003F7334  lh    $v1, 0x2A4($a1)
     003F7338  lh    $v0, 0x294($a1)
     003F733C  slt   $v0, $v0, $v1
     003F7340  bnez  $v0, . + 4 + (0x2 << 2)
     003F7344  nop
     003F7348  addiu $a2, $zero, 0x1        <- one shared assignment for both paths
     003F734C  jr    $ra
     003F7350  daddu $v0, $a2, $zero

   Six shapes tried (if/else, goto, two sequential ifs, a disjunction with the load
   inside it, default-one inverted, early return): the plain if/else if-converts to
   44 bytes with `xori`, and the ones that keep the branch return in $v0 with the
   pointer in $v1 instead of retail's $a2 accumulator. The sibling
   func_L18_003F7470 matched with the lesson "write the success path inside the
   test and the default as the trailing return" - the same inversion here gives
   52 bytes but still $v0 returns (u1/u3 in the working notes).
   Next idea: force the accumulator's identity, e.g. a shape where both assignments
   write one variable read after the merge. */
