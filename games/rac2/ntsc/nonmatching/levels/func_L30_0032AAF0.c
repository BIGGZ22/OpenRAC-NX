/* Not a C row: catalogue defect. The 20-byte extent at 0x0032AAF0 is `addiu $sp,$sp,0x30`
   / `nop` / `addiu $sp,$sp,0x30` / `nop` / `sh $zero,0x1C($a0)` - the epilogue-fragment
   shape of func_L00_00341798 and func_L04_0030CC70 (level 30 is one of the levels with a
   catalogued entry that starts early): no prologue and no `jr $ra` in the whole extent.
   Needs the real body's start from the maintainers. */
