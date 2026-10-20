/* from src/core/00115200.c: not a C candidate at all - the retail body is the
 * SDK's handwritten memcmp, and symbol_addrs/usa/symbol_addrs.txt already
 * names it: `memcmp = 0x00115340; // type:func size:0x80 - name (v2.00
 * Promises or SDK library match), digest match`.
 *
 * Evidence from the bytes (tools/check.py could not be reached because no C
 * source shape produces them):
 *   - 0x78830000 / 0x78A20000 are LQ $v1,0($a0) / LQ $v0,0($a1): 128-bit quad
 *     loads the EE backend only emits from TImode objects;
 *   - 0x704344C9 and 0x710753A9 are MMI (opcode 0x1C) compare/pack ops
 *     (pceqb/pcpyld class), which cc1 never emits from C, and whose whole
 *     purpose here is to test 16 bytes in one go;
 *   - the tail is the classic byte loop with `bne $a2,$a3` against a
 *     materialised 0xFFFFFFFF (-1) and `subu $v0,$v1,$v0` in the jr slot.
 * The frame is 16-byte aligned with an early `sltiu $a2,0x10` guard and an
 * alignment test on `(a0|a1) & 0xF`: an n-byte vector compare, not something
 * a C compiler schedules.
 *
 * Recommendation (maintainer decision, not mine to make): ASM_FUNC, on the
 * same evidence the core syscall stubs were marked with (no C form reaches
 * these instructions). Same for the rest of the SDK string/vector library in
 * this range.
 */
