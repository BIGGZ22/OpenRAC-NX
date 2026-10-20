/* Not a function row: not attempted.
 *
 * The 20-byte extent is five words of orphaned epilogue residue - `addiu $sp,$sp,0x190`,
 * `nop`, `addiu $sp,$sp,0x150`, `nop`, `addiu $sp,$sp,0x40` - with no `jr`, `jal` or branch,
 * so no return path exists to write in C. It is the same catalogue defect as
 * nonmatching/levels/func_L00_0035CC98.c (12 bytes, three words), only longer: the restores
 * name three different frames (0x190, 0x150, 0x40) whose bodies end elsewhere.
 *
 * What would unblock it: an extent fix in the catalogue / tools/levels.py - fold the row into
 * the preceding body or carry it as data.
 */
