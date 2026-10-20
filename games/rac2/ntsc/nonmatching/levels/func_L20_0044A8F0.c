/* Near miss from src/levels/20_yeedil/L20_00449178-g8.c: 388 bytes, retail 408.
   The attempt belongs in the -G8 unit: the body reaches D_L20_001B2444,
   D_L20_001B2448 and D_L20_001B244C through $gp.

   The two clamps are the retail's shape instruction for instruction: the
   pi/4 constant (0x3F490FDC) and its negation (0xBF490FDC), the
   `swc1` in the branch delay slots, the bc1f / b / bc1tl / bc1tl / bc1fl
   sequence and the two `mov.s $f0,$f20` (the function returns the float
   the second func_00282DF0 call gave - the caller compares it with 0.1f).
   The calls, their argument setup and the epilogue match too.

   Two differences:
     1. the 16-byte copies (the wall of this level, see
        func_L20_0044A838 and func_L20_00449FD0): the retail materialises
        both addresses (`lq $v0,0x0($s2) ; sq $v0,0x0($s3)` with $s2 =
        state+0x180 and $s3 = arg0+0x10 kept in callee-saved registers),
        this cc1 folds the offsets into the operands;
     2. the float comparisons: the retail emits the signalling forms
        `c.lt.s` and this cc1 the ordered forms `c.olt.s` for the same C
        (a > pi/4, a < -pi/4, v > pi/4, v < 0).  The operand order and
        the branch structure are the retail's, only the ordered flag
        differs - not measured further.

   Because the copies fold, this cc1 needs one callee-saved register less
   (the retail keeps the state+0x180 pointer in $s2), so the frame is 0x60
   where the retail's is 0x70. */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef int TI128 __attribute__((mode(TI)));


extern float D_L20_001B2444 __attribute__((sda));

extern float D_L20_001B2448 __attribute__((sda));

extern float D_L20_001B244C __attribute__((sda));

extern float func_00282DF0(void *);

extern void func_002AAC80(void *, float, float, float, float);

extern void func_002AFDD8(void *, void *, void *, int);

extern void func_L07_00321D68(void *, void *);

float func_L20_0044A8F0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *v = state + 0x180;
    char *src = arg0 + 0x10;
    float b0[4];
    float b1[4];
    float b2[4];
    float f;
    float a;

    func_00282CF0(b0, state + 0x190, state + 0x180);
    b2[0] = func_00282DF0(state + 0xF0);
    f = func_00282DF0(b0);
    func_002AAC80(b2, f, D_L20_001B2444 * 0.00027777778f,
                  D_L20_001B2444 * 0.00027777778f,
                  D_L20_001B2448 * 0.016666668f);
    func_00282F20(state + 0xF0, b0, b2[0]);
    *(TI128 *)src = *(TI128 *)v;
    func_L07_00321D68(arg0, state + 0xF0);
    *(TI128 *)v = *(TI128 *)src;
    func_002AFDD8(arg0, b1, state + 0xF0, 0);
    a = b1[1] * -D_L20_001B244C;
    *(float *)(state + 0x1A8) = a;
    *(float *)(state + 0x1AC) = b1[0] * D_L20_001B244C;
    if (a > 0.78539824f) {
        *(float *)(state + 0x1A8) = 0.78539824f;
    } else if (a < -0.78539824f) {
        *(float *)(state + 0x1A8) = -0.78539824f;
    }
    a = *(float *)(state + 0x1AC);
    if (a > 0.78539824f) {
        *(float *)(state + 0x1AC) = 0.78539824f;
    } else if (a < 0.0f) {
        *(float *)(state + 0x1AC) = 0.0f;
    }
    return f;
}

/* WAVE 3 (2026-10-20, worker 09, unit src/levels/20_yeedil/L20_00449178-g8.c).
   The attempt above is unchanged and is still the best one.  Measured
   size: 388 bytes against the retail's 408 - twenty bytes short.

   THE c.olt.s / c.lt.s RESIDUE IS NOT A DIFFERENCE.  The R5900's
   c.lt.s has a different bit pattern from the general MIPS one
   (opcodes/mips-opc.c: "The r5900 has a different bit pattern for the
   c.lt.s instruction"): c.lt.s assembles as 0x46000034 on this target,
   and 0x46000034 is the pattern the general MIPS calls c.olt.s.  The
   retail's word here is 0x46011834 (its listing prints "c.lt.s") and this
   cc1's is 0x460d6034 - both are & 0xffe007ff == 0x46000034, the same
   instruction.  rabbitizer prints "c.olt.s" for both sides of the
   tools/check.py diff, so the wave-2 note that read the retail's own
   listing against rabbitizer's output mistook a naming difference for a
   code difference.  There is no signalling compare to chase.

   What is left is the 16-byte copy: the retail materialises both
   addresses (addiu $s2, $s4, 0x180 for the source and $s3 for
   arg0+0x10) and keeps them in callee-saved registers across the calls,
   which is why its frame is 0x70 with five saved registers where this
   cc1's is 0x60 with four.  The mechanism (the CSE's find_best_addr cost
   tie) is written up in full in nonmatching/levels/func_L20_0044A838.c
   (wave 3). */
