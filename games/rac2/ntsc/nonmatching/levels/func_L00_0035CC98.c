/* Not C, and not a function row: not attempted.
 *
 * src/levels/shared/L00_00358D98.c. The 12-byte extent is three words of
 * orphaned epilogue residue: `addiu $sp,$sp,0x70` / `nop` / `addiu $sp,$sp,0x50`
 * (level 00 at 0x0035CC98, level 01 at 0x00353070, level 02 at 0x00363C10).
 * There is no `jr`, no `jal` and no branch in the row, so no return path exists
 * to write in C, and a scan of the whole text finds nothing that branches or
 * jumps into any of the three word boundaries (checked in levels 00 and 01 -
 * scratch/whotargets.py). Both fragments restore frames (0x70 and 0x50) that
 * belong to bodies which already end with their own `jr $ra; nop` earlier: the
 * last complete body ends at 0x0035CC90 (level 00), and the next prologue,
 * `addiu $sp,$sp,-0x40`, starts at 0x0035CCA8.
 *
 * What would unblock it: the extent belongs in a catalogue/tools-levels fix -
 * fold the 12 bytes into the preceding row, or carry the row as data (the
 * frame sizes are the only identification the bytes give). Spending C budget
 * here cannot pay.
 */
