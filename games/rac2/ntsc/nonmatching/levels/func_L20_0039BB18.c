/* func_L20_0039BB18 (724 bytes, level 20 only) -- NEAR MISS, not integrated.

   This body belongs to the -G8 unit: it reads the float D_L20_001AEC68 through
   $gp (-0x388($gp) = 0x1AEC68), so it must be built with -G8.  The attempt
   below is in the -G8 unit (src/levels/20_yeedil/L20_002DEBE0-g8.c) and the
   marker is back in place there.

   WHAT IS MATCHED.  tools/check.py compiles the whole unit, so the verdict is
   for the file; for this body the opcode-by-opcode comparison against the
   retail is 176 of the retail's 181 instructions, and the frame, the branch
   structure, the FPR allocation (f20..f25 in the retail's order and offsets)
   and the whole tail are the retail's.  The five remaining differences:

   1. THE 128-BIT ZERO STORE (the known compiler-profile wall, 4 bytes).
      The retail writes the zero of the 16-byte local with a materialised
      register
          por $v0, $zero, $zero ; sq $v0, 0x10($sp)
      while this cc1's rac2_fold_zero_ti_store folds it to
          sq $zero, 0x10($sp)
      The pass folds whenever the zero register is dead after the store, and
      it is: the next use of $v0 is the `lq $v0,0x10($sp)` of the copy that
      follows, which REWRITES the register (the pass stops at the first write).
      The por survives only when the zero feeds two or more stores; the retail
      body has one.  Measured spellings that all fold: `v.q = 0`, `v = {0}`,
      `*(volatile TI128 *)&v = 0`, the same store through a pointer, and a
      union with a TI128 member (that spelling is the one below; it is what
      puts the float at sp+0x18 and keeps the buffer address out of a
      callee-saved register, as the retail has it).  The same wall is recorded
      for func_L20_0044AB68, func_L13_0040A2C8, func_L00_002A75D8 and
      func_L00_003ACC88.

   2. THE REGISTER TIE (2 instructions + the frame's save order).  The retail
      keeps the argument in $s2 and the integer result of func_002A7820 in $s1;
      this attempt keeps them the other way round ($s1 and $s2).  The
      instructions are the same, only the two callee-saved registers are
      swapped, so the prologue's save order (sd $s2 / sd $s1) and every use
      differ.  The priorities that decide this are the number of references to
      the parameter (9 in the retail, 7 here, because item 3 removes two) and
      the number to the local (3 in both).

   3. THE LOAD OF THE FIELD AT +0xB8 (1 instruction).  The retail loads it
      twice - once in the branch where the counter is already 11 or more
      (0x0039BB74) and once in the block that calls func_00282E80
      (0x0039BB88) - and this attempt loads it once, hoisted into the
      counter block and shared by both uses.  The two uses are the same
      expression at the tree level, so the dom pass hoists the load into the
      common dominator and the cse merges them.  Spellings tried that do not
      stop it: the second use through a `u8 *` cast, through an array
      subscript, and the two uses in different source blocks.

   4. THE COUNTER'S 64-BIT COPY (2 instructions).  The retail reads the u16
      counter, copies it into a 64-bit register and compares THAT copy
      (`daddu $v1,$v0,$zero ; addiu $v0,$v0,0x1 ; sltiu $v1,$v1,0xB`); this
      attempt compares the loaded value itself (`addiu $v1,$v0,0x1 ;
      sltiu $v0,$v0,0xB`).  A u64 counter gives the 64-bit compare but also a
      64-bit increment (`daddiu` + `andi $v0,$v0,0xFFFF`), and an explicit
      `(unsigned long long)` cast of a u32 is folded away again.  What the
      retail needs is a 32-bit increment of the loaded word and a 64-bit
      comparison of a copy of it.

   5. THE CLAMP'S RESULT REGISTER (2 instructions).  The retail computes the
      clamped value into $f0 (`sub.s $f0,$f2,$f1` / `add.s $f0,$f2,$f1`,
      the register the product g*K had), this attempt computes it in place in
      $f2.  The two variables of the attempt (the sum v and the result w) are
      coalesced into one register by the reload; the retail's two are not.

   WHAT THE BODY IS.  The emitter of the yeedil level: it advances the
   object's counter at +0x96, decelerates the float at the object's +0x68
   towards 0 or 1 with a step of 0.016666667f (1/60), and when the object
   reaches 0 it hands over to func_0029F338.  Then it builds a velocity
   vector (a random radius, the angle from func_002A7940, sin/cos through
   func_00283180/func_00283198) and a copy of it, and calls
   func_00282CC0 and func_002C3150 with them, then func_L00_002E6CF0 with
   func_L20_0039BEC8 as the callback.

   THE PROTOTYPES THIS BODY NEEDS (the ones in the unit are guesses made for
   the other bodies and are wrong here):

       extern int   func_00282948(u32);       - the retail passes a 32-bit value
       extern float func_00282E80(u32, u32);   - both arguments are 32-bit
       extern float func_002AA2E8(float *, float, float);
       extern void  func_0029F338(void *);
       extern int   func_002A7820(int, int);
       extern float func_002A7940(void);
       extern void  func_00282CC0(void *, void *, u32);   - the third is 32-bit
       extern void  func_002C3150(void *, void *, void *, void *, int, int, float);
       extern void  func_L00_002E6CF0(void *, void *);

   The argument itself is 32-bit in the retail: `daddu $s2,$a0,$zero` is the
   zero-extension of a u32 parameter, and every use of it is a 32-bit add
   (`addiu $a0,$s2,0x10`), while the field at +0x68 is loaded with `lw` (a
   32-bit handle) and used as a float pointer through `daddu $a0,$s0,$zero`.
   The float constants are 0.1f, 0.3f, 0.5f, 2.0f, 1.0f, 10.0f and
   0.016666667f (0x3C888889, NOT 0.016666666f which is 0x3C888888).

   THE BRANCH SHAPES THAT DECIDE THE BODY (all measured here):

   - the counter: `*(u16 *)(arg0+0x96) = n + 1;` is unconditional and the
     store sits in the delay slot of the `bnez`, so the source increments
     first and compares the old value;
   - the comparison with 0.1f is `c.lt.s $f1,$f0` + `bc1f` to the else
     block, which is what an `if (f > 0.1f) { A } else { B }` with A inline
     gives; the two blocks A and B are the then and else parts of ONE if,
     not two gotos - with gotos the compiler puts B inline and A out of line,
     the other way round;
   - the min of the two random z values needs the negated comparison:
     `if (!(vel.f[2] > rr2)) { } else { vel.f[2] = rr2; }` gives
     `c.lt.s $f1,$f0` + `bc1tl` with the store in the annulled delay slot,
     while the plain `if (rr2 < vel.f[2])` gives `bc1fl`;
   - the 16-byte copy is `lq $v0,0x10($sp) ; sq $v0,0x0($sp)`, which the
     union spelling gives (two locals at different offsets do not fold).

*/

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int TI128 __attribute__((mode(TI)));

typedef union {
    TI128 q;
    float f[4];
} L20_0039BB18_V4;

extern float D_L20_001AEC68 __attribute__((sda));

extern int func_00282948(u32);
extern float func_00282E80(u32, u32);
extern float func_002AA2E8(float *, float, float);
extern void func_0029F338(void *);
extern float func_002A7878(float, float);
extern int func_002A7820(int, int);
extern float func_002A7940(void);
extern float func_00283180(float);
extern float func_00283198(float);
extern void func_00282CC0(void *, void *, u32);
extern int func_002A77E0(int);
extern void func_002C3150(void *, void *, void *, void *, int, int, float);
extern void func_L00_002E6CF0(void *, void *);
extern void func_L20_0039BEC8(void *);

void func_L20_0039BB18(void *arg0)
{
    L20_0039BB18_V4 pos;
    L20_0039BB18_V4 vel;
    u32 s0;
    float t;
    float g;
    float angle;
    float rnd01;
    float rnd;
    float zero_arg;
    u32 n;
    int m;
    int r;
    float r2;
    float rr;
    float rr2;

    s0 = *(u32 *)((u32)arg0 + 0x68);
    if (func_00282948(s0 + 4) != 0)
        goto pathA;
    n = *(u16 *)((u32)arg0 + 0x96);
    *(u16 *)((u32)arg0 + 0x96) = n + 1;
    if ((unsigned long long)n < 11)
        goto check;
    if (*(u16 *)(*(u32 *)((u32)arg0 + 0xB8) + 0x34) & 1)
        goto pathA;
check:
    if (func_00282E80((u32)arg0 + 0x10, *(u32 *)((u32)arg0 + 0xB8) + 0x10) > 0.1f) {
pathA:
        zero_arg = 0.0f;
        func_002AA2E8((float *)s0, zero_arg, 0.016666667f);
        if (*(float *)s0 == 0.0f) {
            func_0029F338(arg0);
            goto end;
        }
    } else {
        func_002AA2E8((float *)s0, 1.0f, 0.016666667f);
    }

common:
    g = D_L20_001AEC68;
    {
        float v = *(float *)(s0 + 0xC) + g * 0.016666667f;
        float w;
        *(float *)(s0 + 0xC) = v;
        if (v > 1.0f) {
            w = v - 1.0f;
        } else if (v < -1.0f) {
            w = v + 1.0f;
        } else {
            goto skip;
        }
        *(float *)(s0 + 0xC) = w;
    }
skip:
    rnd = func_002A7878(0.1f, 0.3f);
    m = func_002A7820(30, 60);
    vel.q = 0;
    r2 = func_002A7878(0.5f, 2.0f);
    vel.f[2] = -r2 * 0.016666667f;
    pos.q = vel.q;
    rnd01 = func_002A7878(0.0f, 1.0f);
    angle = func_002A7940();
    vel.f[0] = func_00283180(angle) * rnd01;
    vel.f[1] = func_00283198(angle) * rnd01;
    vel.f[2] = 0.0f;
    rr = func_002A7878(1.0f, 10.0f);
    vel.f[2] = rr;
    rr2 = func_002A7878(1.0f, 10.0f);
    if (!(vel.f[2] > rr2)) {
        ;
    } else {
        vel.f[2] = rr2;
    }
    func_00282CC0(&vel, &vel, arg0 + 0x10);
    m &= 0xFF;
    r = func_002A77E0(255);
    func_002C3150(&vel, &pos, 0x60107080, m, r & 0xFF, 0, rnd);
    func_L00_002E6CF0((void *)func_L20_0039BEC8, arg0);
end:
    return;
}
