/* 0 of 8 places match (not attempted): the row holds two MMI words,
   `.word 0x7A620000` at +0x60 and `.word 0x7C820000` at +0x64 (lq/sq family,
   rabbitizer reports INVALID), so this cc1 cannot emit it. The rest of the row is
   ordinary C: spawn func_3113F0(sub-object id) into $s0, bail when it is null, run
   func_32AA20(s0, self), raise 0x34 bit 0 + clear 0x31, copy the parent's 0x10
   timer into 0x98, take the third argument into the sub-object's 0x10 and call
   func_31B920(s0, 5, -1). Not C. */
