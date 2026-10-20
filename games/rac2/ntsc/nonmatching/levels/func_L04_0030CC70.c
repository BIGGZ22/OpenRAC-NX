/* Not a C row: catalogue defect. The 12-byte extent at 0x0030CC70 is `addiu $sp,$sp,0x30`
   / `nop` / `addiu $sp,$sp,0x80` - the same epilogue-fragment shape as func_L00_0031AFF0,
   no prologue and no `jr $ra`. Needs the real extent from the maintainers. */
