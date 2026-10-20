/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point. Removed from src/ 2026-10-13.

   Wave 9 measurement of the plain-C route: `typedef int TI128 __attribute__((mode(TI)))`
   reaches `lq`/`sq` but comes out 68 bytes against retail's 72, because this cc1 folds
   the constant 0xF0 into the `sq` operand (`sq $vf02,0xF0($s0)`) where retail materialises
   the pointer first (`addiu $s0,$s0,0xF0`) and copies with a zero offset.  Measured with
   about twenty spellings - the TI pointer as a variable, the expression written out at
   the use, the pointer assigned before and after the call, the base typed `int *`,
   `short *`, `float *` and `TI128 *`, the TI type with `aligned(1)` and `aligned(16)`,
   an aligned aggregate (that gives `ld`/`sd`), a TI/float union (also `ld`/`sd`),
   `memcpy`/`__builtin_memcpy` (unaligned word moves) - and with the SDK compiler as
   well as the game's: both fold the offset the same way.  The pointer is materialised
   only when it is used twice or the offset is a variable. */

func_L02_003CADA8(char *arg0, char *arg1) {
    char *p = *(char **)(arg0 + 0x68);
    func_002A7750(arg0, 7, -1);
    qcopy(p + 0xF0, arg1);
}
