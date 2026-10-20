/* Near miss from src/levels/20_yeedil/L20_00449178-g8.c: 300 bytes, retail 308.
   The attempt belongs in the -G8 unit (the body reaches D_L20_001B23F8
   through $gp).

   Three differences:
    * the 128-bit copy into the object: retail materialises the destination
          addiu $v1, $s1, 0x10 ; lq $v0, 0x0($s2) ; sq $v0, 0x0($v1)
      while this cc1 folds the offset into the store
          sq $v0, 0x10($s1)
      (the second copy, into state, has plain-register addresses on both sides
      and matches exactly);
    * the two float arguments land in the callee-saved FPRs the other way
      round: the retail has $f21 = the first float and $f20 = the second,
      this cc1 has $f21 = the second and $f20 = the first.  The frame, the
      FPR saves and the restores are the retail's;
    * the instruction order around the state stores: the retail materialises
      the 1/60 constant (0x3C888889) late, just before the multiply that
      uses it, and computes the product into $f0; this cc1 hoists the
      constant to the top of the block and keeps the product in $f2.

   Measured this wave (2026-10-20): the func_0029EFD0(0x12EE) call, the
   null test, the func_002B37E8 call, the two func_002A7940 calls (the first
   one's result is stored at obj+0xF0, the second one's at obj+0xF4 - the
   delay slot of the second call holds the first store), the func_002833F0
   call, the 0x100 flag, the D_L20_001B23F8 multiply, the state stores, the
   func_002A7878(10.0f, 20.0f) call and the 1/3600 multiply are all the
   retail instructions.  The float constants: 0.016666668f (1/60,
   0x3C888889) and 0.00027777778f (1/3600, 0x3991A2B4); the last product
   multiplies the RESULT of func_002A7878, not the earlier f12/60.

   THE RETURN VALUE, found in this wave: the retail's body ends with
       daddu $v0, $s1, $zero
   before the epilogue, so the function returns the object pointer, and the
   prototype is not void.  The earlier attempt had no return value at all,
   which cost one instruction (4 bytes).

   THE 128-BIT COPY WALL: the mechanism, the one measured escape (an offset
   outside [-32768, 32752] makes the reload materialise the address, and gas
   rewrites `addu rd, rs, imm` into the retail's three-operand addiu) and the
   thirty spellings that all fold are written up in
   nonmatching/levels/func_L20_0044A838.c.

   History: an earlier attempt in this file was 300 bytes; it had the fourth
   argument as an int, the second and third as void *, and called
   func_002A7878 with (20.0f, 20.0f) - the retail passes 10.0f in f12 and
   20.0f in f13. */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef int TI128 __attribute__((mode(TI)));

extern float D_L20_001B23F8 __attribute__((sda));

extern void *func_0029EFD0(int);
extern void func_002B37E8(u8 *, u8 *);
extern float func_002A7940(void);
extern void func_002833F0(void *, void *);
extern float func_002A7878(float a, float b);

char *func_L20_00449FD0(char *arg0, char *arg1, char *arg2, char *arg3,
                        float f12, float f13) {
    char *obj;
    char *state;

    obj = func_0029EFD0(0x12EE);
    if (obj != 0) {
        func_002B37E8((u8 *)obj, (u8 *)arg0);
        *(TI128 *)(obj + 0x10) = *(TI128 *)arg1;
        *(float *)(obj + 0xF0) = func_002A7940();
        *(float *)(obj + 0xF4) = func_002A7940();
        func_002833F0(obj + 0xC0, obj + 0xF0);
        *(u8 *)(obj + 0x20) = 1;
        *(u16 *)(obj + 0x34) |= 0x100;
        *(float *)(obj + 0x2C) =
            *(float *)(*(u32 *)(obj + 0x24) + 0x24) * D_L20_001B23F8;
        state = *(char **)(obj + 0x68);
        *(TI128 *)state = *(TI128 *)arg2;
        *(int *)(state + 0x18) = (int)arg3;
        *(float *)(state + 0x14) = f12 * 0.016666668f;
        *(float *)(state + 0x1C) = f13;
        *(int *)(state + 0x20) = 0x3C;
        *(float *)(state + 0x10) =
            func_002A7878(10.0f, 20.0f) * 0.00027777778f;
    }
    return obj;
}

/* WAVE 3 (2026-10-20, worker 09, unit src/levels/20_yeedil/L20_00449178-g8.c).
   The attempt above is unchanged and is still the best one.  Measured
   size: 300 bytes against the retail's 308 - eight bytes short, which is
   exactly the retail's copy:
       addiu $v1, $s1, 0x10 ; lq $v0, 0x0($s2) ; sq $v0, 0x0($v1)
   where this cc1 folds the offset into the store (sq $v0, 0x10($s1)).
   The mechanism (the CSE's find_best_addr cost tie) is written up in full
   in nonmatching/levels/func_L20_0044A838.c (wave 3).

   The two float arguments still land the other way round ($f21 = the
   second, $f20 = the first, where the retail has $f21 = the first and
   $f20 = the second), and the 1/60 constant is hoisted to the top of the
   block where the retail materialises it late, just before the multiply.
   Both are scheduling differences; -fno-schedule-insns was measured on the
   unit and it breaks the two bodies already matched in it. */
