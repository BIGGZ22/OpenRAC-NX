/* Not attempted: the row carries four 128-bit quad words where ordinary C would copy two
 * sixteen-byte blocks - 0x78A20000 and 0x78820000 (opcode 0x1E, LQ) into 0x7CE20000 and
 * 0x7CA20000 (opcode 0x1F, SQ) - after a halfword flag update at 0x34 and two field copies
 * from the object at +0x68. The quad copies sit between the field work and the two `jal`s, and
 * rabbitizer prints all four as `.word # INVALID`.
 *
 * The repository's own drafts show this is not structurally out of reach but is where
 * candidates die: nonmatching/levels/func_L00_00338E90.c records "quadword copies are present,
 * but entry scheduling and saved-register allocation differ" and
 * nonmatching/levels/func_L00_003EE288.c records quadword/TI-copy addressing differences.
 *
 * What would unblock it: a quad-copy plan (the -mastra-quad-saves route / TImode spelling)
 * shared across the quad rows before any budget is spent per row.
 */
