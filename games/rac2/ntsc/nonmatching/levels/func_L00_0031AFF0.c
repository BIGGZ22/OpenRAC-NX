/* Not a C row: catalogue defect. The 12-byte extent at 0x0031AFF0 is two function
   epilogue tails - `addiu $sp,$sp,0x20` / `nop` / `addiu $sp,$sp,0x20` - with no prologue,
   no `jr $ra` and no body. Nothing to write as C; the maintainers need the real extent
   (the neighbouring functions' epilogues suggest the split is short by one function). */
