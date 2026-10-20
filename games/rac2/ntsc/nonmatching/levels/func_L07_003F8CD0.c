/* func_L07_003F8CD0, 188 bytes, 1 placement.  NOT ATTEMPTED - screened out (MMI class).
   `.word 0x700014A9` (pcpyld $v0,$zero,$zero) at +0x30, followed by
   `.word 0x7FA20030` (sq $v0,0x30($sp)): the same 128-bit zero materialisation the
   func_L00_003ACC88 near miss records, folded to `sq $zero` by rac2_fold_zero_ti_store because
   the register is dead afterwards.  The rest of the body is ordinary calls (func_307940 =
   func_0029FCC0, func_2E45F0 = func_00282F20, func_L07_003F6528, func_2E2100 =
   func_L04_0030C4C0, func_L07_003F65E8).  Near miss deposited instead of C, as the brief asks
   for the MMI class. */
