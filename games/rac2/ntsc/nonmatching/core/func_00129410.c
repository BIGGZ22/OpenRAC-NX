/* from src/core/00128D58.c: not a C candidate - a dead VU0 remnant, not a function.
 *
 * The row is 64 bytes at 0x00129410, the last twelve of them the
 * linker's 0xCDCDCDCD fill:
 *
 *   0x00129410  F8860000  sdc2  $6, 0x0($a0)      COP2 (VU0)
 *   0x00129414  00000000  nop
 *   0x00129418  27BD00A0  addiu $sp, $sp, 0xA0
 *   0x0012941C  00000000  nop
 *   0x00129420  F88A0000  sdc2  $10, 0x0($a0)     COP2 (VU0)
 *   0x00129424  00000000  nop
 *   0x00129428  E4800008  swc1  $f0, 0x8($a0)
 *   0x0012942C  00000000  nop
 *   0x00129430  F8890000  sdc2  $9, 0x0($a0)      COP2 (VU0)
 *   0x00129434  00000000  nop
 *   0x00129438  00000000  nop
 *   0x0012943C  00000000  nop
 *   0x00129440  7CA30000  ext   $v1, $t1, 0       (a move; decoded by hand, splat prints a word)
 *   0x00129444  CDCDCDCD  ...                     linker's fill to 0x00129450
 *
 * The first wall alone is enough: the three `sdc2` stores are COP2 (VU0)
 * instructions, which no C compiler emits - the same wall as
 * func_001291A0, func_001244B8 and func_L03_002EFB60. All three write to
 * the same address, 0x0($a0), which no compiler emits either.
 *
 * The row is not a function body either: it has no `jr $ra`, and nothing
 * in the boot reaches it. A scan of both text sections (core.text
 * 0x00115200-0x00133B90 and .text 0x0026EA00-0x00352200) finds no jump
 * or branch target at 0x00129410, no `lui` of its high half and no
 * `addiu`/`ori` of its low half, and the word 0x00129410 occurs nowhere
 * in the boot image, so there is no pointer to it either. The same scan
 * finds the four `jal`s of the neighbour func_00129368 (0x0027BB48,
 * 0x002E6AD8, 0x002E9900, 0x002F5B94), so it does see calls.
 *
 * The row before it, func_00129368 (168 bytes), is a live VU0 helper and
 * is already ASM_FUNC; the row after it, func_00129450 (592 bytes), is an
 * ordinary function with a normal prologue and a 0xB0 frame. What sits
 * between them is 52 bytes of unreachable VU0 code with the linker's
 * fill after it.
 *
 * Recommendation: keep the INCLUDE_ASM marker. ASM_FUNC would be wrong
 * here - it counts the function as finished, and this is not a function.
 * The catalogue-side fix (a tools/levels.py split that tells the 52
 * bytes of code from the 12 bytes of fill) is a maintainer's call.
 */
