/* func_L08_00425398 (176 bytes, 1 place) - NOT C with this compiler (same wall as
 * func_L08_00425328: the EE COP1 word 0x46010028 at 0x00425434).
 *
 * No candidate C is deposited because the function cannot pass whatever the rest spells: its
 * 0x00425434 word is 0x46010028 - COP1 fmt=S funct=0x28, fd=$f0 fs=$f0 ft=$f1, the same
 * one-instruction float operation as func_L08_00425328's 0x4600A028, and cc1 has no max.s
 * pattern (measured: the ternary and the if-assignment both emit c.olt.s + a likely branch +
 * mov.s, eight bytes longer).  See nonmatching/levels/func_L08_00425328.c for the probe.
 *
 * The rest of the body, for whoever retakes it once that op is available:
 *   func_L08_00425398(char *a0):
 *     q = *(char **)(a0 + 0x68)                       (lw $s0,0x68($a0))
 *     aim = a0 + 0x10                                 (addiu $a0,$a0,0x10 ; lq/sq of 16 bytes
 *                                                      into the frame at $sp+0, both ends)
 *     f0 = *(f32 *)(sp + 8) + 10.0f                   (lwc1 $f0,0x8($sp) ; mtc1 0x41A0 ; add.s)
 *     *(f32 *)(sp + 8) = f0
 *     func_L08_00425328(sp, 5.0f, *(f32 *)(q + 0x178) - 3.0f, *(f32 *)(q + 0x90))   (f12 = f0+5.0f)
 *     *(f32 *)(q + 0x1C4) = <that call's result>      (swc1 in the jal delay slot)
 *     if (func_00278E78(...) == 0) *(f32 *)(q + 0x1C4) += 2.0f
 *     *(f32 *)(q + 0x1C8) = max(D_00189EA8, *(f32 *)(q + 0x1C4) + 6.5f)   <-- the 0x46010028
 * The last store's operand is the 0x189EA8 constant (lui 0x19) and the sum uses 6.5f (0x40D0).
 * Everything except that one word is ordinary C; a max.s-capable cc1 should land it in one
 * attempt once the frame layout above is written.
 */
