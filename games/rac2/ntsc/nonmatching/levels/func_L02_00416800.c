/* func_L02_00416800 (68 bytes, 2 places) -- NEAR MISS, not integrated.
   state: 68 bytes, 9 words differ, and every one is a register name; the instruction
   sequence is retail's exactly (lbu/bne/bnez/lwc1/mtc1/c.eq.s/bc1tl with the 1 in the
   slot, jr with the accumulator in the slot).
   retail: state byte in $v0, the constant 1 in $v1, the record pointer in $a1, the
           accumulator in $a2 (`daddu $v0,$a2,$zero` at the return).
   ours:   the same four roles one register lower ($v1, $v0, $a0, $a1).
   Five shapes tried in one call (pointer declared inside the block or at the top, a
   single disjunction with the assignment inside it, both assignments split, constant
   on the left of the tests): the nesting ones all give the identical 9 words, the
   disjunction grows to 64 bytes. A pure allocation tie. */
