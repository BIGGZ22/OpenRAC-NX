/* Near miss from src/levels/20_yeedil/L20_00449178-g8.c: 380 bytes, retail 392.
   The attempt belongs in the -G8 unit (the body reaches D_L20_001B2470 and
   D_L20_001B246C through $gp), so the body must move out of
   src/levels/20_yeedil/L20_00449178.c into the -G8 sibling.

   Three differences:
    * the 128-bit copy into the object: retail materialises the destination
          addiu $s2, $s3, 0x10 ; lq $v0, 0x0($s6) ; sq $v0, 0x0($s2)
      while this cc1 folds the offset into the store
          sq $v0, 0x10($s3)
      (the second copy, into state, has plain-register addresses on both sides
      and matches exactly);
    * the two nops the retail's division padding needs before the div.s;
    * the order of the loads before the obj+0x2C store: the retail does
          lwc1 $f1, 0x3480($gp) ; lwc1 $f0, 0x24($v1) ; lw $s0, 0x68($s3)
      and this cc1 does the state load first.

   THE THREE STATE STORES, corrected in this wave.  A delay slot runs BEFORE
   its call, so the retail's
       jal f_0032F7E0 ; addiu $s1, $s0, 0x10
       ...
       jal f_0032F718 ; swc1 $f0, 0x24($s0)
       sw $zero, 0x30($s0)
       daddu $a0, $s4, $zero
       jal f_003033B8 ; swc1 $f0, 0x28($s0)
   means
       state + 0x24 = func_002A7940()          (the first call's result)
       state + 0x28 = func_002A7878(pi/60, pi/30)
       state + 0x30 = 0
       state + 0x34 = 1.0f / func_00283CE0(arg4)
   and not the pi/60 value in both 0x24 and 0x28, as the earlier attempt
   had.  The earlier attempt also passed a third (float) argument to
   func_L11_00460258, which the retail does not: the retail's call is
   f_0044B5E8(dst, the $gp word) with no float argument.

   Measured this wave (2026-10-20): the frame (0x70), the eight saved
   GPRs, the FPR save, the func_0029EFD0(0x12F7) call, the null test, the
   func_002B37E8 call, the D_L20_001B2470 multiply, the state stores, the
   two func_002A7878 results stored at state+0x24 and state+0x28, the
   func_00283CE0 call, the 1.0f divide, the four vector calls and the
   func_002E5DA0 call are the retail instructions.  The float constants:
   0.052359886f (pi/60, 0x3D567752) and 0.10471977f (pi/30, 0x3DD67752).

   THE 128-BIT COPY WALL: the mechanism, the one measured escape (an offset
   outside [-32768, 32752] makes the reload materialise the address, and gas
   rewrites `addu rd, rs, imm` into the retail's three-operand addiu) and the
   thirty spellings that all fold are written up in
   nonmatching/levels/func_L20_0044A838.c.  Keeping the destination in a
   variable that lives across the calls (`dst = obj + 0x10`, used again for
   func_L11_00460258) does give the retail's register allocation - $s2 =
   obj + 0x10 - but the offset is still folded into the store. */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef int TI128 __attribute__((mode(TI)));

extern float D_L20_001B2470 __attribute__((sda));
extern int D_L20_001B246C __attribute__((sda));

extern void *func_0029EFD0(int);
extern void func_002B37E8(u8 *, u8 *);
extern float func_002A7940(void);
extern float func_002A7878(float a, float b);
extern float func_00283CE0(int);
extern void func_00282D30(void *, void *, float);
extern void func_00282CC0(void *, void *, void *);
extern void func_00282CF0(void *, void *, void *);
extern float func_00282DF0(void *);
extern void func_00282F20(void *, void *, float);
extern void func_L11_00460258(char *, int);
extern void func_002E5DA0(int, int, void *, int);

void func_L20_0044B100(char *arg0, char *arg1, char *arg2, char *arg3, int arg4,
                        float f12) {
    char *obj;
    char *state;
    char *dst;
    TI128 buf;
    float r;

    obj = func_0029EFD0(0x12F7);
    if (obj != 0) {
        func_002B37E8((u8 *)obj, (u8 *)arg0);
        dst = obj + 0x10;
        *(TI128 *)dst = *(TI128 *)arg1;
        *(float *)(obj + 0x2C) =
            *(float *)(*(u32 *)(obj + 0x24) + 0x24) * D_L20_001B2470;
        state = *(char **)(obj + 0x68);
        *(TI128 *)state = *(TI128 *)arg2;
        *(float *)(state + 0x20) = f12;
        *(float *)(state + 0x24) = func_002A7940();
        *(float *)(state + 0x28) = func_002A7878(0.052359886f, 0.10471977f);
        *(int *)(state + 0x30) = 0;
        r = func_00283CE0(arg4);
        *(float *)(state + 0x34) = 1.0f / r;
        func_00282D30(&buf, arg2, (float)(arg4 >> 1));
        func_00282CC0(&buf, &buf, arg1);
        func_00282CF0(state + 0x10, arg3, &buf);
        r = func_00282DF0(state);
        func_00282F20(state + 0x10, state + 0x10, r);
        func_L11_00460258(dst, D_L20_001B246C);
        func_002E5DA0(0, 0, obj, 0x12F7);
    }
}

/* WAVE 3 (2026-10-20, worker 09, unit src/levels/20_yeedil/L20_00449178-g8.c).
   The attempt above is unchanged and is still the best one.  Measured
   size: 380 bytes against the retail's 392 - twelve bytes short.  The
   retail materialises the copy's destination address
       addiu $s2, $s3, 0x10 ; lq $v0, 0x0($s6) ; sq $v0, 0x0($s2)
   where this cc1 folds it (sq $v0, 0x10($s3)); the mechanism (the CSE's
   find_best_addr cost tie) is written up in full in
   nonmatching/levels/func_L20_0044A838.c (wave 3).

   The two nops the retail's division padding needs before the div.s and
   the order of the loads before the obj+0x2C store (the retail does
   lwc1 $f1,0x3480($gp) ; lwc1 $f0,0x24($v1) ; lw $s0,0x68($s3), this
   cc1 loads the state pointer first) are the other two differences; they
   were not chased in this wave because the copy blocks the function
   anyway. */
