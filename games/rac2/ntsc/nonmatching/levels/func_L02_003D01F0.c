/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point. Removed from src/ 2026-10-13.

   Wave 9 measurement of the plain-C route: `mode(TI)` reaches `lq`/`sq` but comes out
   68 bytes against retail's 72, because this cc1 folds the constant 0xB0 into the `sq`
   operand where retail materialises the pointer first (`addiu $s0,$s0,0xB0`) and copies
   with a zero offset - the same wall as its twin func_L02_003CADA8, measured there. */

func_L02_003D01F0(char *arg0, char *arg1) {
    char *self = arg0;
    char *p = *(char **)(self + 0x68);
    func_002A7750(self, 4, -1);
    qcopy(p + 0xB0, arg1);
}
