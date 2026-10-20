/* from src/core/00131780.c: not attempted; the row holds two functions.
 *
 * 0x00131D98 itself is an 80-byte loop driver - func_0011FC48(), then the
 * code at 0x0026EDE8 through a `jalr`, func_00131CB8(), and then
 * `while (1) { func_0011AEA0(0); func_0011AEA0(2); }` at +0x20..+0x48, which
 * is what symbol_addrs names `main`. At +0x50 a new frame opens
 * (`addiu $sp,$sp,-0x40`, s0-s4 and $ra saved) and a different 576-byte body
 * runs to the end of the row.
 *
 * The four catalogue places want all 656 bytes at one address, so no single
 * C body can match; a tools/levels.py split at 0x00131DE8 unblocks both
 * halves.
 */
