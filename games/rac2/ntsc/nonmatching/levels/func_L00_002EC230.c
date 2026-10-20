/* from src/levels/shared/L00_002E5918.c: not a function. The 20-byte row at
 * L00:0x002EC230 spans the linker's padding between two bodies: three `addiu
 * $sp,$sp,N; nop` epilogue fragments (N = 0x30, 0x80, 0xC0), no prologue and no
 * `jr $ra`. The body before it ends at 0x002EC22C (`jr $ra; addiu $sp,$sp,0x20`)
 * and the next function starts at 0x002EC248. Same defect class as the ten rows
 * found in 0032EB28.c (docs/COMPILER-NOTES.md, "Catalogue rows that are not
 * functions"): a tools/levels.py extent fix, no C can produce these bytes.
 * Data for the maintainers, nothing to attempt here. */
