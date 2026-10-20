/* func_L20_00440198 (508 bytes, level 20): NOT ATTEMPTED in this wave; the blocker is
 * measured and recorded.
 *
 * The body has one 128-bit copy: `addiu $v1,$s1,0x10 ; lq $v0,0x0($a1) ; sq $v0,0x0($v1)`.
 * The SOURCE is a plain register ($a1, a parameter) and compiles as it is; the
 * DESTINATION is $s1 + 0x10, and this cc1 folds the constant offset into the `sq`
 * (`sq $v0,0x10($s1)`), so the retail's `addiu` is lost and the body is four bytes short.
 *
 * The one escape of that wall that exists (measured in this wave, see
 * nonmatching/levels/func_L20_004499E8.c) is a volatile pointer OBJECT:
 * `TI128 *volatile d = (TI128 *)(s1 + 0x10); *d = *(TI128 *)a1;` gives
 * `sw ; lq ; lw ; sq $v0,0($reg)` - the retail's zero-displacement `sq` - but it costs the
 * stack store and the reload, eight bytes over the retail's single `addiu`. So the body
 * would be 516 bytes against 508.
 *
 * The rest of the body (the table walk with `beql`/`bgez`, the `movz` clamp, the three
 * calls func_0032F1F8, func_0032F5B0, func_0027AD0 and the func_00302260 call) was not
 * written: the 128-bit wall makes the body unmatchable anyway.
 */
