/* Near miss from src/levels/20_yeedil/L20_0039C448-g8.c (the -G8 unit).

   The body has the retail size (380 bytes) and the prologue, the first block
   and the epilogue match the retail instruction for instruction.  Three
   instructions are scheduled differently:

     +0x80  retail:  daddu a0,s0 / lwc1 f1,-0x2F0(gp) / addiu a1,zero,1 /
                     lwc1 f0,0x24(v0) / addiu a2,zero,-1 / mul.s f0,f0,f1
              here:   daddu a0,s0 / addiu a1,zero,1 / addiu a2,zero,-1 /
                     lwc1 f0,0x24(v0) / lwc1 f1,D_L20_001AED00(gp) /
                     mul.s f0,f0,f1
     +0xB4  retail:  daddu a2,zero,zero / daddu a3,zero,zero / mov.s f12,f21 /
                     jal f_0033ECF8 / mov.s f13,f21
              here:   daddu a2,zero,zero / mov.s f12,f21 / mov.s f13,f21 /
                     jal f_0033ECF8 / daddu a3,zero,zero
     +0xC4  retail:  lw v1,0x60(s1) / bnez v1 / lui v0,0x19 / lw v1,-0x3F50(v0)
              here:   lw v0,0x60(s1) / bnez v0 / lui v0,0x19 / lw v1,-0x3F50(v0)

   So the retail interleaves the setup of the f_0032F5B0 arguments with the
   float multiply, fills the delay slot of the f_0033ECF8 call with the
   `mov.s f13,f21` instead of the `daddu a3,zero,zero`, and keeps the word
   read from 0x60(s1) in $v1.

   What did work (and is in the attempt):
   - the prologue, the frame (0x30) and the two saved FPRs ($f21 for the zero,
     $f20 for the result) match exactly, so the zero of the f_0033ECF8 call
     and the comparison is a variable and not a literal.
   - D_L20_001AED00 carries `sda` (the retail reaches the float through $gp at
     0x1AEFF0-0x2F0) and D_0018C0B0 carries `nosda` (the retail reaches the word
     through $at: lui v0,0x19 / lw v1,-0x3F50(v0)).
   - the float at 0x2C is `*(f32 *)((char *) *(int *)(arg0 + 0x24) + 0x24)`: the
     retail's `lw v0,0x24(s0)` / `lwc1 f0,0x24(v0)` needs the +0x24 inside the
     load.
   - the first f_0032F5B0 call takes (arg0, 1, -1), not (arg0, 5, -1): the
     retail materialises $a1=1 and $a2=-1 before the float multiply.
   - the flags at 0x34 are written twice: first &= 0xFFFB, later
     (| 0x41) & 0xEFFF.

   Tried: the call before the float store, the constant in a local variable,
   the constant as the first operand of the multiply, the pointer to the float
   in a local variable, the order of the stores in the block, D_0018C0B0 with
   and without `nosda`, and the state pointer loaded before or after the
   first call.  The three differences above are what is left.    With the attempt in src/ (the unit's default flags):
     20_yeedil: instruction at +0x80 differs
     func_L20_0039E5F0: 0 of 1 places match
*/
#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

extern void func_00330A68(void *, int);

extern void func_0032F5B0(void *, int, int);

extern void func_0033ECF8(void *, int, int, int, float, float);

extern int func_00331BB8(void *, int, int);

extern float func_00331C18(void *, int);

/* 0x1AEFF0 - 0x2F0 : the float the retail body reaches through $gp. */
extern float D_L20_001AED00 __attribute__((sda));

/* 0x190000 - 0x3F50 : the word the retail body reaches through $at. */
extern int D_0018C0B0 __attribute__((nosda));

void func_L20_0039E5F0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(u8 *)(arg0 + 0x20) == 0) {
        if (*(f32 *)(state + 0x20) == 0.0f) {
            *(u16 *)(state + 0x24) = 0x1E;
            *(f32 *)(state + 0x20) = 30.0f;
        }
        func_00330A68(arg0, 0xCBC);
        *(u16 *)(arg0 + 0x32) = 0xFF;
        *(u16 *)(arg0 + 0x34) &= 0xFFFB;
        *(f32 *)(arg0 + 0x2C) = *(f32 *)((char *) *(int *)(arg0 + 0x24) + 0x24) * D_L20_001AED00;
        *(int *)(state + 0x190) = 1;
        *(int *)(arg0 + 0x98) = 0;
        *(u16 *)(arg0 + 0x34) = (*(u16 *)(arg0 + 0x34) | 0x41) & 0xEFFF;
        func_0032F5B0(arg0, 1, -1);
    }
    {
        f32 zero = 0.0f;
        f32 r;

        func_0033ECF8(arg0, *(int *)(state + 0x80), 0, 0, zero, zero);
        if (*(int *)(state + 0x60) == 0) {
            *(int *)(state + 0x60) = D_0018C0B0;
        }
        r = func_00331C18(arg0, func_00331BB8(arg0, 0x10000, 0));
        {
            f32 d = *(f32 *)(state + 0x20) - r;

            if (d <= zero) {
                *(f32 *)(state + 0x20) = d;
                func_0032F5B0(arg0, 5, -1);
            }
        }
        if (5.0f <= r) {
            func_0032F5B0(arg0, 4, -1);
        }
    }
    *(u8 *)(arg0 + 0xA8) = 0xFF;
}

/* ------------------------------------------------------------------
   Second attempt (worker 09, level-20 wave); the C in this file is that
   second attempt, the first one is described above.  It fixes one of the
   three differences of the attempt above: the delay slot of the
   f_0033ECF8 call now holds `daddu a3,$zero,$zero`, as the retail has.

   What is new in this attempt: the last two arguments of f_0033ECF8 are
   the two floats and the last int is the fourth argument
   (`extern void func_0033ECF8(void *, int, int, float, float, int);` and
   `func_0033ECF8(arg0, *(int *)(state + 0x80), 0, zero, zero, 0);`).
   The register assignment is the same (the ints in $a0-$a3, the floats in
   $f12-$f15), but the argument setup follows the source order, so the
   `daddu a3,$zero,$zero` lands last and goes into the delay slot.

   What is still different (the retail on the right):
     0039E670  addiu $a1,$zero,0x1 | lwc1 $f1,-0x2F0($gp)
     0039E674  addiu $a2,$zero,-0x1 | addiu $a1,$zero,0x1
     0039E67C  lwc1  $f1,0($gp)   | addiu $a2,$zero,-0x1
     0039E6C4  lw    $v0,0x60($s1) | lw $v1,0x60($s1)
     0039E6C8  bnez  $v0         | bnez $v1
     0039E70C  daddu $a0,$s0      | swc1 $f0,0x20($s1)
     0039E710  swc1  $f0,0x20($s1)| daddu $a0,$s0

   1. The f_0032F5B0 arguments are interleaved with the float multiply in
      the retail (a0, the constant, a1, the value, a2); this attempt keeps
      the three argument setups together and the float code after them.
      The retail's order is what the scheduler produces when the constant
      is loaded before the value; this attempt loads the value first
      because the source multiplies `value * constant`.  Spelling it
      `constant * value` does not change it (the operands are swapped back
      in the reload).
   2. The word read from 0x60(state) is in $v1 here and in $v0 in the
      retail, which uses $v0 for the address of D_0018C0B0.
   3. In the last block the store to 0x20(state) is in the delay slot of
      the bc1f in the retail and the call's first argument setup is in it
      here - the scheduler's choice of the delay slot.

   Also measured: -fno-schedule-insns for this body (in a unit of its own,
   src/levels/20_yeedil/L20_0039E5F0-g8.c with the row
   `src/levels/20_yeedil/L20_0039E5F0-g8.c  -O2 -G8 -ffunction-sections
   -fno-schedule-insns` in config/cflags.txt) makes it worse: with the
   flag the argument setup keeps the source order and the registers of the
   flag block change too.

   The attempt belongs in the -G8 unit (it reaches D_L20_001AED00 through
   $gp). */

#include "include_asm.h"

/* This unit is built with -G8 (the body reaches a global through $gp) and with
   -fno-schedule-insns: the retail body of func_L20_0039E5F0 interleaves the setup
   of the f_0032F5B0 arguments with the float multiply, which the default
   scheduler does not reproduce.  The flag is per file, so this function lives in
   a unit of its own. */


extern void func_00330A68(void *, int);

extern void func_0032F5B0(void *, int, int);

extern void func_0033ECF8(void *, int, int, float, float, int);

extern int func_00331BB8(void *, int, int);

extern float func_00331C18(void *, int);

/* 0x1AEFF0 - 0x2F0 : the float the retail body reaches through $gp. */
extern float D_L20_001AED00 __attribute__((sda));

/* 0x190000 - 0x3F50 : the word the retail body reaches through $at. */
extern int D_0018C0B0 __attribute__((nosda));

void func_L20_0039E5F0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(u8 *)(arg0 + 0x20) == 0) {
        if (*(f32 *)(state + 0x20) == 0.0f) {
            *(u16 *)(state + 0x24) = 0x1E;
            *(f32 *)(state + 0x20) = 30.0f;
        }
        func_00330A68(arg0, 0xCBC);
        *(u16 *)(arg0 + 0x32) = 0xFF;
        *(u16 *)(arg0 + 0x34) &= 0xFFFB;
        *(f32 *)(arg0 + 0x2C) = *(f32 *)((char *) *(int *)(arg0 + 0x24) + 0x24) * D_L20_001AED00;
        *(int *)(state + 0x190) = 1;
        *(int *)(arg0 + 0x98) = 0;
        *(u16 *)(arg0 + 0x34) = (*(u16 *)(arg0 + 0x34) | 0x41) & 0xEFFF;
        func_0032F5B0(arg0, 1, -1);
    }
    {
        f32 zero = 0.0f;
        f32 r;

        func_0033ECF8(arg0, *(int *)(state + 0x80), 0, zero, zero, 0);
        if (*(int *)(state + 0x60) == 0) {
            *(int *)(state + 0x60) = D_0018C0B0;
        }
        r = func_00331C18(arg0, func_00331BB8(arg0, 0x10000, 0));
        {
            f32 d = *(f32 *)(state + 0x20) - r;

            if (d <= zero) {
                *(f32 *)(state + 0x20) = d;
                func_0032F5B0(arg0, 5, -1);
            }
        }
        if (5.0f <= r) {
            func_0032F5B0(arg0, 4, -1);
        }
    }
    *(u8 *)(arg0 + 0xA8) = 0xFF;
}

/* ------------------------------------------------------------------
   Wave 21, worker 08.  What this wave measured on the same body.

   The attempt below (worker 09's second one) is 380 bytes, the
   retail's size, and matches the retail from the prologue to 0x39E66C
   and from 0x39E684 to the end.  Seven instructions differ, in three
   groups:

     +0x80  retail:  daddu a0,s0 / lwc1 f1,-0x2F0(gp) / addiu a1,zero,1 /
                      lwc1 f0,0x24(v0) / addiu a2,zero,-1 / mul.s f0,f0,f1
               here:   daddu a0,s0 / addiu a1,zero,1 / addiu a2,zero,-1 /
                      lwc1 f0,0x24(v0) / lwc1 f1,D_L20_001AED00(gp) /
                      mul.s f0,f0,f1
     +0xC4  retail:  lw v1,0x60(s1) / bnez v1 / lui v0,0x19 / lw v1,-0x3F50(v0)
               here:   lw v0,0x60(s1) / bnez v0 / lui v0,0x19 / lw v1,-0x3F50(v0)
     +0xE4  retail:  bc1f +0x14 / swc1 f0,0x20(s1) / daddu a0,s0,zero
               here:   bc1f +0x14 / daddu a0,s0,zero / swc1 f0,0x20(s1)

   The first group is the scheduler: the retail interleaves the setup of
   the f_0032F5B0 arguments with the float multiply (the constant's
   load before a1, the value's load before a2), the attempt keeps the
   three argument setups together.  The second group is the register
   the word read from 0x60(state) lands in ($v1 in the retail, $v0 in
   the attempt).  The third group is which instruction the delay slot
   of the bc1f takes (the store in the retail, the call's first
   argument in the attempt).

   What was measured in this wave (about 25 compiles):

   - the default flags are the right ones for this body:
     `-fno-schedule-insns` gives 21 differing instructions,
     `-fno-schedule-insns2` 16, `-O1`/`-O3` more.
   - the operands of the multiply (`value * D_L20_001AED00` and
     `D_L20_001AED00 * value`) do not change the schedule (7 and 8
     differing); the value or the constant in a local makes it worse
     (30 and 37) because the load is hoisted to the top of the
     function.
   - the store's target (`*(f32 *)(arg0 + 0x2C)` and
     `((f32 *)arg0)[0x2C]`), the comparison's polarity
     (`d <= zero` and `!(d > zero)`), the word read from 0x60(state)
     through a pointer variable, the word in a local, the comparison
     `== 0` against `!x`, and the declaration place of `zero` and `r`
     (in the block or at the top of the function) all change nothing:
     7 differing instructions in every case.
   - `sda` on D_0018C0B0 instead of `nosda` makes it much worse (43):
     the retail reaches that word through $at (`lui v0,0x19` +
     `lw v1,-0x3F50(v0)`), so `nosda` is right.

   Note for the next worker: this cc1 is a C89 compiler.  A
   declaration after a statement is a parse error ("parse error before
   'zero'"), so the locals of a block have to be declared at the top
   of that block.

   The attempt below is worker 09's second one, unchanged: it is the
   closest one.
*/
