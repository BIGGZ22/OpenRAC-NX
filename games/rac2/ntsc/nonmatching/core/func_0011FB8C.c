/* from src/core/0011FA70.c: the row's first 12 bytes are not this function.
 *
 *   0x11FB8C  0xCDCDCDCD          the linker's fill word (rac1-decomp's
 *                                 src/libgcc/README.md documents the same
 *                                 value in its two retail gaps);
 *   0x11FB90  addiu $sp,$sp,0x20  the surviving delay slot of the `jr $ra`
 *                                 of a function the linker dead-stripped -
 *                                 the frame it pops (0x20) and the ordering
 *                                 match rac1's strip_dead model exactly;
 *   0x11FB94  nop                 that remnant's alignment nop.
 *
 * The real body runs from 0x11FB98 to 0x11FC48 (176 bytes, `lui $a3,0x1A;
 * addiu $sp,$sp,-0x30`), ends in `jr $ra; addiu $sp,$sp,0x30`, and is a
 * teardown routine: when D_001A7000 == -1 and D_001A7004 != 0 it walks the
 * callback table at D_001A7000 for the first null entry, then calls the
 * entries below it back down to index 1 (`jalr`, no arguments), popping $ra,
 * $s1 and $s0 from a 0x30 frame. C can match that body; it cannot emit the
 * three words before it, so the row as catalogued is unreachable from src/.
 *
 * What would unblock it: a tools/levels.py split that starts the row at
 * 0x11FB98 (176 bytes) and gives 0x11FB8C..0x11FB94 to the stripped sibling
 * or to a filler row, as the catalogue already does elsewhere for remnants.
 * No compile was spent: the offset is arithmetic, not a source shape.
 */
