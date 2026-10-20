/* 0 of 8 places match (not attempted): five MMI words in the row -
   `0x7A620000`, `0x7E020000`, `0x7A820000`, `0x7C620000`, `0x7A220000` (lq/sq
   family, rabbitizer reports INVALID), so this cc1 cannot emit it. The rest is
   ordinary C: bail when the owner is null, reach the 0x68 sub-object, run
   func_2EEFF8(sub+0x30, first, third, 0.5f) and store func_2EF158(first, third)
   into the sub-object's 0x3C. Not C. */
