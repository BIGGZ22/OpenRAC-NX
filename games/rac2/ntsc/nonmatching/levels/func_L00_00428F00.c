/* func_L00_00428F00 (232 bytes, 27 places) - TI128 destination wall, plus one quad pair
 * that may be reachable.  Diagnosis only, no C measured this wave.
 *
 * Readable: $v1 = *(void**)(a0+0x70), the state word *(int*)(D_L00_00189E20+0x2294) gates a
 * 0x2B / 0x23-0x25 dispatch, then the body copies a 16-byte subrecord at 0xB8 and a 16-byte
 * record at +0x30 out of D_L00_001C8C80 (the lq/sq words 0x7A420000 / 0x7C620000,
 * 0x7A220000 / 0x7C820000, 0x78620000 / 0x7CA20000, 0x78620000 / 0x7C820000), and finally
 * calls func_428588(a0), stores 0x2D to *(int*)($s0+0x314) and calls func_428EB0(a0) with
 * func_2DA1E0(s1+0x30, 0.5f) and sw $zero,0x80($v0).
 * The pairs all use a materialised pointer with offset 0 (addiu $v1,$a0,0x30 then
 * lq $v0,0x0($v1)), which is the shape that did NOT survive in func_L00_004282F0 - so the
 * same constant-displacement fold is expected here.  Worth retrying only with a new lever
 * for that fold.
 */
