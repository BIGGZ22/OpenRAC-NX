/* func_L03_003C4368 (252 bytes, 2 places) - decoded, no C measured; this is the row to try
 * the $gp-float finding on.
 *
 * Structure: return unless *(u8*)(a0+0x20) is 4 or 2; $s1 = *(void**)(a0+0x68); when
 * *(void**)(s1+0x198) is nonzero, call func_322158 on it and, if that answers nonzero,
 * func_2EEEF0(sp, *(f32*)(r+0x18)) and return early when 0x3ADA740F compares less than that
 * (c.olt.s + bc1tl with the frame restore in the delay slot).  Then
 * func_2EEFE0(sp, a0+0x10, a1), $s0 = s1 + 0x1B0, and two calls whose float arguments come
 * straight from $gp: func_2EF298(sp, sp, (*(f32*)$gp + same) * 0x3991A2B4) with
 * sw $zero,0x8($sp), then func_2EEFB0(s0, s0, sp) and
 * func_3217B8(s0, *(f32*)$gp2 * 0x3C888889).
 * The two $gp loads are 0xF18(gp) and 0xF14(gp) - float sda reads - exactly the case worker
 * 10 measured as needing -g8 PLUS -mno-split-addresses on the unit's cflags line to come out
 * bare instead of lui $at + lwc1.  Try that combination first.
 */
