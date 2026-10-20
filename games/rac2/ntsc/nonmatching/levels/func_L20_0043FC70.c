/* func_L20_0043FC70 (892 bytes, level 20): NOT ATTEMPTED in this wave; the blocker is
 * measured and recorded.
 *
 * Two 128-bit stores with a constant destination offset:
 *   `lw $v1,0x2290($v0) ; addiu $v1,$v1,0x10 ; lq $v0,0x0($v1) ; sq $v0,0x0($a1)` and
 *   `addiu $v0,$s1,0xF0 ; sq $zero,0x0($v0)`.
 * This cc1 folds both offsets into the `sq` (`sq $v0,0x10($v1)` and `sq $zero,0xF0($s1)`),
 * so each of them loses the retail's `addiu` and the body is eight bytes short.
 *
 * The one escape of that wall that exists (measured in this wave, see
 * nonmatching/levels/func_L20_004499E8.c) is a volatile pointer OBJECT, which reaches the
 * retail's zero-displacement `sq` at the price of a stack store and a reload - eight bytes
 * over per destination, so 908 bytes against 892 here.
 */
