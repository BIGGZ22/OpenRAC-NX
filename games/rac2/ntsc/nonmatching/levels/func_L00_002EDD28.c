/* from src/levels/shared/L00_002E5918.c: twin of func_L00_002EDCF0 one float later -
 * returns the $gp float at gp-0x76D8 (0x001A7918) times the argument. Retail, 16
 * bytes: lwc1 $f0,-0x76D8($gp); nop; jr $ra; mul.s $f0,$f0,$f12. The -g8 sibling and
 * `extern float D_001A7918 __attribute__((sda));` reproduce the load and the mul.s
 * operand order, but the pinned cc1 gives the compact three-instruction form (12
 * bytes) and drops retail's `nop` before `jr $ra` (docs/COMPILER-NOTES.md wall).
 * Marked G8. */
extern float D_001A7918 __attribute__((sda));

float func_L00_002EDD28(float a) {
    return D_001A7918 * a;
}
