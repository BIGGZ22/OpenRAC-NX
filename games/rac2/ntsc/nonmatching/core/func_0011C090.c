/* from src/core/0011BF18.c: not attempted in this run; the double -> decimal-exponent
   formatter over the libgcc soft-float helpers, reconstruction below */
/*
 * 0x11C090..0x11C1F8, 360 bytes. Real body, ordinary C: no COP2/MMI/`sync`, no `.word`
 * words, no `jr $ra` - it leaves through `j func_11C7E8` TWICE, each after restoring
 * $ra/$s2/$s1/$s0 itself, so both are sibling tail calls and both work in the sdk profile.
 *
 * Profile: `addiu $sp,$sp,-0x40` with `sd $s2,0x20` / `sd $s0,0x0` / `sd $s1,0x10` /
 * `sd $ra,0x30` - 16-byte slots, so the same `sdk -O2 -G2 -ffunction-sections` family as
 * its neighbours 0011BF18.c / 0011C8B0.c.
 *
 * Callees, all already named in this repo's own libgcc files (src/core/001213B8.c):
 *   func_122A98 dpsub, func_122B00 dpmul, func_122DA8 dpdiv, func_123028 dpcmp,
 *   func_1212C8 __fixunsdfdi. Add func_11C000 (integer -> string) and func_11C7E8
 *   (string, int), called with the .rodata run at 0x13A9C0 / 0x13A9C8 / 0x13A9D0
 *   (`addiu $a0,$s2,-0x5640` with $s2 = 0x140000).
 *
 * Inline double constants, read as bits: `ori $a1,$zero,0x8048 ; dsll32 $a1,$a1,15` is
 * 0x4024000000000000 = 10.0; `ori $a1,$zero,0xFFC0 ; dsll32 $a1,$a1,14` is
 * 0x3FF0000000000000 = 1.0. The other thresholds are LOADED with `ld` from 0x13A9D8,
 * 0x13A9E0 and 0x13A9E8 - a double table in the same .rodata run as _printf's, which is
 * the same placement problem the __divdi3/__moddi3 pair has.
 *
 * Sign handling calls a hook: `lui $v1,0x13 ; lw $v0,0x4698($v1) ; jalr $v0` with
 * `addiu $a0,$zero,0x2D` ('-'), its result discarded. 0x134698 is the first of the two
 * function pointers that src/core/0011C8B0.c's neighbour func_0011C8D8 installs at
 * 0x13469C - the pair is one unit.
 *
 * Control flow read off the branch targets (x in $s0, exponent counter in $s1):
 *     if (dpcmp(x, 0.0) < 0) { x = dpsub(0.0, x); hook('-'); }
 *     if (dpcmp(x, C1) < 0)
 *         while (dpcmp(x, C2) < 0) { x = dpmul(x, 10.0); exp--; }
 *     else
 *         while (dpcmp(x, C3) >= 0) { x = dpdiv(x, 10.0); exp++; }
 *     x = dpmul(x, C4);
 *     func_11C7E8(0x13A9C0, func_11C000(__fixunsdfdi(x)));
 *     if (exp < 0) return func_11C7E8(0x13A9D0, exp);
 *     return func_11C7E8(0x13A9C8, exp);
 * The two loops are laid out with the body first and the test at the bottom, the inner
 * `while` entered by a forward `b` to its test - a shape worth checking before touching it.
 */
