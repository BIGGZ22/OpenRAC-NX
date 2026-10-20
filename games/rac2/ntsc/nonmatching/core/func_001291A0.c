/* from src/core/00128D58.c: not a C candidate - VU0 macro-mode body.
 *
 * 116 bytes, of which 19 of 29 instructions are COP2 (opcode 0x12) macro ops
 * with rs in 0x11..0x1F, plus one `ldc2 $5,0x0($t0)` at +8 whose address is
 * built by `lui $t0,0x13; addiu $t0,$t0,0x7EA0` (= 0x00137EA0, the constant
 * pool this routine reads). What is left is a `bnez $a3` and a `jr $ra; nop`.
 * cc1 emits no COP2 macro instructions from C at all, so no source shape
 * reaches these bytes.
 *
 * It is a real function, not a catalogue artifact: `jal 0x001291A0` occurs 3
 * times in the boot (0x00129254, 0x001292FC, 0x001293A4 - all inside the
 * neighbouring VU0 helpers), and its neighbours func_00129218, func_001292C0
 * and func_00129368 are already ASM_FUNC in the same file for the same
 * reason.
 *
 * Recommendation (maintainer decision): ASM_FUNC.
 */
