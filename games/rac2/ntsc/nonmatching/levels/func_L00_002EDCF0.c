/* from src/levels/shared/L00_002E5918.c: returns the $gp float at gp-0x76DC
 * (0x001A7914, a resident symbol: same address in every level) times the argument.
 * Retail, 16 bytes: lwc1 $f0,-0x76DC($gp); nop; jr $ra; mul.s $f0,$f0,$f12.
 * With `extern float D_001A7914 __attribute__((sda));` (and the -g8 sibling unit)
 * the load and the mul.s operand order match retail exactly, but the pinned cc1
 * emits the compact three-instruction form (lwc1; jr $ra; mul.s - 12 bytes): the
 * retail `nop` before `jr $ra` is the wall documented in docs/COMPILER-NOTES.md
 * ("a nop before jr $ra ... those bodies do not reproduce"). Marked G8. */
extern float D_001A7914 __attribute__((sda));

float func_L00_002EDCF0(float a) {
    return D_001A7914 * a;
}
