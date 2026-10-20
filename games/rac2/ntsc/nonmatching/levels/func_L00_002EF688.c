/* from src/levels/shared/L00_002E5918.c: not a function. The 20-byte row at
 * L00:0x002EF688 is four `addiu $sp,$sp,0x10; nop` pairs of the linker's padding
 * after the function whose epilogue ends at 0x002EF684 (`jr $ra` at 0x002EF67C,
 * `addiu $sp,$sp,0x10` in the slot); the row itself has no prologue and no
 * `jr $ra`. The next function starts at 0x002EF6A0. Defect class of
 * docs/COMPILER-NOTES.md, "Catalogue rows that are not functions": a
 * tools/levels.py fix, no C reaches it. */
