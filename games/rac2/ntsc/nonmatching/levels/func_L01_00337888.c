/* NEAR MISS - wave 11 S4.  428 bytes against retail's 464 (9 instructions short);
   the shape is right (the frame, the calls, the two index computations) but the
   register allocation is not: this build needs one callee-saved GPR more, spills
   the third argument to the stack and takes a third FPR.

   Retail body (level 01, 0x00337888, 464 bytes, 5 places) - the function walks
   the table of 16-byte records the second argument points at, measures the
   record the index selects and writes the result into the caller's record:

     003378D0  v1 = *a1; a0 = v1 - 1
     003378D8  bne $s3,$a0 -> 00337904        ; a3 != n-1 -> skip
     003378E0  v1 = n - 2; v0 = (0 < t0); s2 = 0
     003378EC  b -> 00337904
     003378F0  movz $s2,$v1,$v0              ; t0 <= 0 -> s2 = n-2
     003378F4  bnel $s3,$zero -> 00337908     ; DEAD (the branch above skips it)
     003378F8  addu $s2,$s3,$s5              ; DEAD
     003378FC  addiu $s2,$zero,0x1           ; DEAD
     00337900  slti $v0,$s5,0x1             ; DEAD
     00337904  movn $s2,$a0,$v0              ; t0 > 0 -> s2 = n-1
     00337908  s0 = a3<<4; a1 = s2<<4; s1 = a1p + a3*16 + 0x10
     0033791C  func_002E34A0(v1, a1p + s2*16 + 0x10, s1)
     00337930  f21 = func_002E35A0(v1)
     00337948  func_002E3738(v1, v1, 1.0f)
     0033795C  func_002E34A0(v2, a0 + 0x10, s1)
     00337968  f20 = f20 + func_002E3560(v2, v1)
     00337974  if (f21 < f20) {
     00337984      the same index computation again, with the first index as its
     00337998      "a3" input, then
     003379D8      func_002E34A0(v1, a1p + idx2*16 + 0x10, a1p + idx*16 + 0x10)
     003379DC      f20 = f20 - f21
     003379F0      func_002E3738(v1, v1, 1.0f)
     003379F8  }
     00337A00  func_002E3738(v1, v1, f20)
     00337A14  func_002E3470(a2, v1, a1p + j*16 + 0x10);  return j

   The index the first block computes is dead once "t0 > 0" overrides it with
   n-1, which is why the "a3 == 0 ? 1 : a3 + t0" arm survives in the object as
   unreachable code after an unconditional branch.

   The draft below reaches the right calls and the right arithmetic, but its
   register assignment is one GPR out (it spills the third argument and keeps a
   third FPR) and the two index computations do not come out in the retail's
   shape.  Six compiles.

   Compiler habits measured here (all confirmed on this cc1):

   * A dead store that lands in a branch delay slot is NOT removed: the retail
     keeps "addu $s2,$s3,$s5" and "addiu $s2,$zero,0x1" after an unconditional
     branch, and func_L00_002D1DA0 keeps "addiu $a1,$s1,-0x1" the same way.
     The same store in ordinary code IS removed.
   * `if (c) x = a; else x = b;` comes out as `movz`/`movn` with the "x = 0"
     first: `movz $s2,$v1,$v0` is "if (v0 == 0) s2 = v1" over an earlier
     "s2 = 0".
   * A branch to an unreachable block is still emitted; only the ordinary dead
     code disappears.
*/
#include "include_asm.h"

extern void func_002E34A0(void *, void *, void *);

extern float func_002E35A0(void *);

extern float func_002E3560(void *, void *);

extern void func_002E3738(void *, char *, float);

extern void func_002E3470(void *, void *, void *);

int func_L01_00337888(char *a0, int *a1, char *a2, int a3, int t0, float f12)
{
    char v1[16];
    char v2[16];
    int n = *a1;
    int idx = 0;
    float f20 = f12;
    float f21;

    if (a3 == n - 1) {
        if (t0 > 0) {
            if (a3 == 0)
                idx = 1;
            else
                idx = a3 + t0;
        } else
            idx = n - 2;
    }
    if (t0 < 1)
        idx = n - 1;

    func_002E34A0(v1, (char *)a1 + idx * 16 + 0x10, (char *)a1 + a3 * 16 + 0x10);
    f21 = func_002E35A0(v1);
    func_002E3738(v1, v1, 1.0f);
    func_002E34A0(v2, a0 + 0x10, (char *)a1 + a3 * 16 + 0x10);
    f20 = f20 + func_002E3560(v2, v1);
    if (f21 < f20) {
        int idx2 = 0;
        if (idx == n - 1) {
            if (idx == 0)
                idx2 = 1;
            else
                idx2 = idx + t0;
        }
        if (t0 < 1)
            idx2 = n - 1;
        func_002E34A0(v1, (char *)a1 + idx2 * 16 + 0x10, (char *)a1 + idx * 16 + 0x10);
        f20 = f20 - f21;
        func_002E3738(v1, v1, 1.0f);
    }
    func_002E3738(v1, v1, f20);
    func_002E3470(a2, v1, (char *)a1 + idx * 16 + 0x10);
    return idx;
}
