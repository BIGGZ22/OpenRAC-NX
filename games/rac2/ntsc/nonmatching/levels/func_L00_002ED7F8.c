/* from src/levels/shared/L00_002E5918.c: not a function. The 12-byte row at
 * L00:0x002ED7F8 is `addiu $sp,$sp,0x150; nop; addiu $sp,$sp,0x30` - the linker's
 * padding after the body that ends at 0x002ED7F8 (`jr $ra` at 0x002ED7F0 with
 * `addiu $sp,$sp,0x150` in its slot), cut mid-pair; no prologue and no `jr $ra` in
 * the row. The
 * next function starts at 0x002ED808. Defect class of docs/COMPILER-NOTES.md,
 * "Catalogue rows that are not functions": a tools/levels.py fix, no C reaches it. */
