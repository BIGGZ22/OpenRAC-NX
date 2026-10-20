/* Near miss from src/levels/20_yeedil/L20_00449178-g8.c: 312 bytes, retail 304.
   The attempt belongs in the -G8 unit (the body reaches D_L20_001B2AC8 and
   D_L20_001B23F0 through $gp).

   Two differences:
    * the 128-bit copy: retail materialises the destination address
          addiu $v1, $s2, 0x60 ; lq $v0, 0x0($s1) ; sq $v0, 0x0($v1)
      while this cc1 folds the offset into the store
          sq $v0, 0x60($s2)
      (the load side, arg0 + 0x10 in $s1, is already a plain register and
      matches);
    * one callee-saved register too many (the retail reuses $s0 for the stack
      buffer's address after arg0 dies, this cc1 keeps $s0 for the buffer and
      $s1 for arg0), which also shifts the loop head and makes cc1 emit one
      alignment nop.

   Measured this wave (2026-10-20): the mode test (the *(int*)(state + 0xAC)
   guard), the pointer-hash comparison (((int)arg0 >> 8) & 7 against
   D_L20_001B2AC8 & 7, with the state+0xA4 load in the branch delay slot),
   the func_00277F48 call, the counted loop over D_L20_001C4480 with the
   0x11E0 test, the func_L20_00449AE8 call, the four tail calls and the
   float compare against D_L20_001B23F0 are all the retail instructions.
   The float constant of the func_0032C700 call is 0.666f (0x3F2A7EFA), not
   0.6666667f (0x3F2AAAAB).

   THE CALL ARGUMENTS, corrected in this wave: the retail's second tail
   call is f_003023C0(sp+0x10, arg0+0x10, sp+0) - the same three registers
   as the first one, f_00302290(sp+0x10, arg0+0x10, sp+0) - so the C is
       func_00282CC0((char *)&b, p, (char *)&a);
   and not the argument order the earlier attempt had.  The earlier attempt
   also passed a third (float) argument to func_L11_00460258, which the
   retail does not: the retail's call is f_0044B5E8(dst, the $gp word) with
   no float argument.

   THE 128-BIT COPY WALL: the mechanism, the one measured escape (an offset
   outside [-32768, 32752] makes the reload materialise the address, and gas
   rewrites `addu rd, rs, imm` into the retail's three-operand addiu) and the
   thirty spellings that all fold are written up in
   nonmatching/levels/func_L20_0044A838.c.  In short: the address of a TI
   access is (mem:TI (plus:SI (reg) (const_int))), GO_IF_LEGITIMATE_ADDRESS
   keeps that plus for TImode while the offset is in [-32768, 32752], and
   print_operand_address then folds the constant into the lq/sq.  0x60 is
   inside the range, so the store folds. */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef int TI128 __attribute__((mode(TI)));

extern int D_L20_001B2AC8 __attribute__((sda));
extern float D_L20_001B23F0 __attribute__((sda));
extern char *D_L20_001C4480[] __attribute__((nosda));

extern int func_00277F48(char *a, int b, char *c, int d, float f);
extern void func_L20_00449AE8(char *);
extern void func_L20_00444C18(char *, char *);
extern void func_00282CF0(void *, void *, void *);
extern float func_00282E20(void *vec);
extern void func_00282F70(char *a, char *b);
extern void func_00282CC0(void *, void *, void *);
extern void func_002C9218(char *a, char *b, char *c, float f);

void func_L20_00449BB0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    int count;
    int i;

    if (*(int *)(state + 0xAC) != 0) {
        if ((((int)arg0 >> 8) & 7) == (D_L20_001B2AC8 & 7)) {
            count = func_00277F48(arg0 + 0x10, 0, arg0, 0, 1.5f);
            for (i = 0; i < count; i++) {
                if (*(short *)(D_L20_001C4480[i] + 0xAA) == 0x11E0) {
                    func_L20_00449AE8(arg0);
                    return;
                }
            }
        }
    }
    {
        TI128 a;
        TI128 b;
        char *p = arg0 + 0x10;

        func_L20_00444C18(*(char **)(state + 0xA4), (char *)&a);
        func_00282CF0(&b, p, (char *)&a);
        if (func_00282E20(&b) < D_L20_001B23F0) {
            func_00282F70((char *)&b, (char *)&b);
        }
        func_00282CC0((char *)&b, p, (char *)&a);
        *(TI128 *)(state + 0x60) = *(TI128 *)p;
        func_002C9218(*(char **)(state + 0xA0), p, p, 0.5f);
    }
}

/* WAVE 3 (2026-10-20, worker 09, unit src/levels/20_yeedil/L20_00449178-g8.c).
   The attempt above is unchanged and is still the best one.  Measured
   size: 312 bytes against the retail's 304 - EIGHT BYTES TOO LONG, and
   every difference in the body follows from the 16-byte copy.

   The retail materialises the copy's destination address:
       addiu $v1, $s2, 0x60 ; lq $v0, 0x0($s1) ; sq $v0, 0x0($v1)
   and this cc1 folds it into the store (sq $v0, 0x60($s2)).  The reason is
   the CSE's cost tie, measured and written up in full in
   nonmatching/levels/func_L20_0044A838.c (wave 3): the front end builds
   the MEM's address as a pseudo, the CSE's find_best_addr() replaces it
   with the plus (ADDRESS_COST(REG) = 1 = mips_address_cost(plus(reg, small
   const))), and the reload then prints the offset as the displacement.
   Only a SYMBOL base keeps the address in a register; a MEM base and a
   volatile pointer object were measured and both fail to land.

   THE EXTRA CALLEE-SAVED REGISTER IS A CONSEQUENCE OF THE COPY: with the
   destination address in a register (the retail's shape) the body needs one
   more saved register, so the retail's frame is 0x40 with $s0,$s1,$s2,$ra
   saved, and this cc1's frame is 0x50 with $s0,$s1,$s2,$s3,$ra.  Fixing
   the copy fixes the frame and the save order with it. */
