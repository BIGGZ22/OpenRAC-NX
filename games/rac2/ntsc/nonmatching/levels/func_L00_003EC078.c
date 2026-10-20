/* func_L00_003EC078, 200 bytes, 27 placements.  NOT ATTEMPTED - screened out (MMI class).
   The retail body carries the MMI zero of the func_L00_003ACC88 family at +0x1C
   (`.word 0x700014A9` = pcpyld $v0,$zero,$zero, then `.word 0x7FA20020` = sq $v0,0x20($sp)),
   which rac2_fold_zero_ti_store folds to `sq $zero` whenever the register is dead afterwards -
   the same pinned-chain wall as func_L00_0040B2C0 and func_L00_003ACC88.  The body also moves
   quadwords with `lq $v0,0x20($sp)` at +0x5C and `sq $v0,0x10($sp)` at +0x64, and calls
   func_31A7C0, func_311580 (twice) and func_33C0D0 (three times, func_L00_0033C0D0).
   No C deposited: the family is already documented twice and the wall is compiler-side. */
