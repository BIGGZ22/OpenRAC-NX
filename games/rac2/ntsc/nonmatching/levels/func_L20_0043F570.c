/* func_L20_0043F570 (1788 bytes, level 20): NOT ATTEMPTED in this wave; the blocker is
 * measured and recorded.
 *
 * Two 128-bit stores. The first is a plain register (`sq $zero,0x0($sp)`) and is fine; the
 * second has a constant destination offset (`addiu $v0,$s1,0xF0 ; sq $zero,0x0($v0)`) and
 * this cc1 folds it (`sq $zero,0xF0($s1)`), so the retail's `addiu` is lost.
 *
 * The one escape of that wall that exists (measured in this wave, see
 * nonmatching/levels/func_L20_004499E8.c) is a volatile pointer OBJECT, which reaches the
 * retail's zero-displacement `sq` at the price of a stack store and a reload - eight bytes
 * over.
 */
