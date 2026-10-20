/* Not a C row: catalogue defect. The 20-byte extent at 0x0035B768 is three stack
   restores with their padding - `addiu $sp,$sp,0x50` / `nop` / `addiu $sp,$sp,0x40` /
   `nop` / `addiu $sp,$sp,0x80` - with no prologue, no `jr $ra` and no body. No function
   opens on a stack restore. The bytes before it (0x0035B740..) are the epilogue tail of
   func_L04_0033CB08 (catalogue: level 6, 0x0035B6E8 + 0x7C); in level 5 a real function
   (func_002CFDA8, 0x41C) covers the same address, and level 6's next real entry is
   func_002A8508 at 0x0035B780. Same shape as func_L00_0031AFF0 / func_L00_00358670.
   The maintainers need the real extent of the split - there is nothing to write as C. */
