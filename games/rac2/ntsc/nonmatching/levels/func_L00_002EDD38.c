/* from src/levels/shared/L00_002E5918.c: third twin of the family - returns the $gp
 * float at gp-0x76D4 (0x001A791C) times the argument. Retail, 16 bytes:
 * lwc1 $f0,-0x76D4($gp); nop; jr $ra; mul.s $f0,$f0,$f12. Same wall as
 * func_L00_002EDCF0: the -g8 sibling plus `extern float D_001A791C
 * __attribute__((sda));` match the load and the multiply, but the pinned cc1 emits
 * the compact three-instruction form without retail's `nop` before `jr $ra`
 * (docs/COMPILER-NOTES.md). Marked G8. */
extern float D_001A791C __attribute__((sda));

float func_L00_002EDD38(float a) {
    return D_001A791C * a;
}
