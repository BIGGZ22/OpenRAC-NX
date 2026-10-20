/* 0 of 5 places match - the extent is linker fill, not a function.
   All five placements (04_barlow 0x003A11F8, 06_notak 0x003BEBB8, 08_tabora
   0x0038BE10, 18_damosel 0x00399908, 20_yeedil 0x00393608) hold a chain of
   `addiu $sp,$sp,0x10/0x50 ; nop` pairs with no `jr $ra` and no prologue; the
   next real body is func_L04_003A1240 at 0x003A1240.  A tools/levels.py split
   unblocks it; no C reproduces linker fill. */
