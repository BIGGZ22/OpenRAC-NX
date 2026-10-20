/* 0 of 27 places match - last line: 00_aranos_tutorial: instruction at +0x28 differs
   Size EXACT (152) and every instruction is present: the two 16-byte operand copies are
   retail's lq/sq pair (`lq $v0,0x0($a1)` / `sq $v0,0x0($sp)`, `lq $v1,0x0($a2)` /
   `sq $v1,0x10($sp)`), and each of the four 0x002AA140 blends is `lwc1 $f12,<left>` +
   `lwc1 $f13,<right>` + `mov.s $f14,$f20` + `jal`.
   The lq/sq needs `__attribute__((mode(TI)))` on the copies: a plain 16-byte struct
   assignment (the form this file used to hold) emits ldl/ldr + sdl/sdr pairs and comes out
   188 bytes; the f32-array spell of the same copy comes out 140 (this cc1 then reads both
   operands straight off $sp and keeps no second callee-saved register).
   The remaining gap is that callee-saved pair, not the body: retail copies the destination
   pointer into `$s0` in the prologue (`daddu $s0,$a0,$zero` at +0x28) and materialises the
   right operand's address later, in the first call's delay slot (`addiu $s1,$sp,0x10` at
   +0x3C), so its first store lands at +0x40; here the right operand's address takes `$s0`
   early (+0x28) and `$s1` receives the destination in that delay slot, which shifts every
   store one call later. Measured with ~15 spellings (named f32* locals for both / one /
   none, cast-at-each-use - which ICEs cc1 in convert_move -, a union with a mode(TI)
   member, f32[4] arrays, one f32[8] buffer, a V4 struct, a named destination local, and
   both pointer declaration orders): the body never moves. */

typedef float f32;
typedef unsigned int u128 __attribute__((mode(TI)));

extern float func_002AA140(float, float, float);

void func_L00_0031D670(f32 *out, const f32 *a, const f32 *b, f32 factor) {
    u128 left = *(const u128 *)a;
    u128 right = *(const u128 *)b;
    f32 *lf = (f32 *)&left;
    f32 *rf = (f32 *)&right;

    out[0] = func_002AA140(lf[0], rf[0], factor);
    out[1] = func_002AA140(lf[1], rf[1], factor);
    out[2] = func_002AA140(lf[2], rf[2], factor);
    out[3] = func_002AA140(lf[3], rf[3], factor);
}
