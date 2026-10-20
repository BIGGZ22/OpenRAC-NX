/* Not a function row: not attempted.
 *
 * The 20-byte extent is five words of orphaned epilogue residue - `addiu $sp,$sp,0x50`, `nop`,
 * `addiu $sp,$sp,0x10`, `nop`, `addiu $sp,$sp,0xB0` - with no `jr`, `jal` or branch. Same
 * catalogue defect as nonmatching/levels/func_L00_0035CC98.c and func_L09_00325C58.c: three
 * frames (0x50, 0x10, 0xB0) restored for bodies that end elsewhere.
 *
 * What would unblock it: an extent fix, not C.
 */
