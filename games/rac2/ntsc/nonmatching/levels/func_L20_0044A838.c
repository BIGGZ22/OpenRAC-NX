/* Near miss from src/levels/20_yeedil/L20_00449178-g8.c: 172 bytes, retail 180.
   The attempt belongs in the -G8 unit (the body reaches three $gp-relative
   floats: D_L20_001B2410, D_L20_001B2414, D_L20_001B241C).

   Everything but the 128-bit copy is the retail instruction for instruction:
   the prologue, the three $gp loads, the two calls, the flag test, the
   branch offsets and the epilogue all match.  The copy is the wall: retail
   materialises the two addresses into registers
       addiu $a0, $s0, 0x10 ; addiu $v1, $s1, 0x180 ;
       lq $v0, 0x0($v1) ; sq $v0, 0x0($a0)
   while this cc1 folds the constant offsets into the lq/sq operands
       lq $v0, 0x180($s1) ; sq $v0, 0x10($s0)
   which is 8 bytes short.

   WHERE THE FOLD HAPPENS (measured in the compiler source, 2026-10-20).
   The address of a TI access is (mem:TI (plus:SI (reg:SI base) (const_int
   off)) 0).  Two passes can fold the constant into the lq/sq operand:
     (1) the reload pass.  GO_IF_LEGITIMATE_ADDRESS (config/mips/mips.h)
         accepts plus(reg, const_int) for TImode while
             INTVAL(const) >= -32768 && INTVAL(const) + 16 - 1 <= 32767,
         so strict_memory_address_p() (reload.c) finds the address legal,
         does not reload it into a register, and print_operand_address()
         (config/mips/mips.c) then prints the constant as the displacement.
     (2) the CSE.  With the pointer in a C variable the address is a
         register, and find_best_addr() (gcc/cse.c) replaces it by the
         equivalent plus, because mips_address_cost() gives "register plus a
         small constant" the cost 1 and a plain register the cost 4.
   The retail's own compiler is the same ee-gcc 2.9, so the difference is
   the source, not the flags.

   THE ONE MEASURED ESCAPE, and why it is not usable here.  An offset
   outside [-32768, 32752] is not a legal TImode displacement, so the
   reload pass materialises the address:
       *(TI128 *)(arg0 + 32753) = v;
   compiles to
       addu $4, $4, 32753 ; sq $5, 0($4)
   which is the retail's exact shape - the retail's `addiu $a0, $s0, 0x10`
   is this cc1's `addu $4, $16, 16` (measured: gas 2.9-ee-991111 rewrites
   `addu rd, rs, imm` into the R5900 three-operand addiu encoding 0x25, so
   `addu $4,$16,16` assembles to 0x26040010, the retail's word).  But the
   immediate has to be the real offset, and 0x10 (and 0x180) are inside the
   legal range, so the only way to reach the escape is to bias the base by
   32743, which costs the extra instruction the retail does not have.

   SPELLINGS MEASURED AND FOUND TO FOLD (2026-10-20, this wave): a TI128
   pointer variable; the pointer used twice; the pointer live across a call;
   two C names for one pointer; a struct member; an array index; a union; a
   volatile store; a volatile pointer object (this one reaches a plain
   register address, `sw`/`lw` round trip and a zero-displacement `sq`, but
   costs the stack store and the reload, 8 bytes over); a 64-bit pointer
   (`daddu` plus a `dsll`/`dsra` sign-extension pair); a 16-bit, 8-bit or
   int base; the offset out of range (above); a static __inline__ copy
   helper (2 and 4 parameters); __builtin_memcpy; and the per-file flags
   -g, -mno-split-addresses, -fno-schedule-insns, -fno-schedule-insns2,
   -fno-expensive-optimizations and -O1 (none changes the shape).

   TWO SHAPES THAT DO REACH A ZERO-DISPLACEMENT sq, for the record: a
   pointer that is LOADED from memory (`lw $v1, 0x68($a0) ; lq $v0, 0($a1)
   ; sq $v0, 0($v1)`, the matched func_L08_004356A0), and a pointer that is
   carried by a loop and incremented at the end of the body, where the
   store's address register has no plus in its class at the store (the
   matched func_002E5FE0).  Neither can express "base + 0x10" with an
   addiu.

   History: an earlier attempt in this file was 184 bytes (the copy alone was
   two instructions instead of four); the body below is the 172-byte one,
   which matches the retail instruction for instruction everywhere but the
   copy. */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef int TI128 __attribute__((mode(TI)));

extern float D_L20_001B2410 __attribute__((sda));
extern float D_L20_001B2414 __attribute__((sda));
extern float D_L20_001B241C __attribute__((sda));

extern float func_00283B98(float, float);
extern float func_00283198(float);
extern void func_002A9288(char *, float);
extern void func_L20_0044AA88(char *);

void func_L20_0044A838(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    u16 n = *(u16 *)(arg0 + 0x96);
    float r;

    *(u16 *)(arg0 + 0x96) = n + 1;
    if (*(u8 *)(arg0 + 0x31)) {
        *(TI128 *)(arg0 + 0x10) = *(TI128 *)(state + 0x180);
        r = func_00283B98(*(float *)(state + 0x1A0),
                          D_L20_001B2414 * 0.000290888245f);
        *(float *)(state + 0x1A0) = r;
        r = func_00283198(r);
        *(float *)(arg0 + 0x18) = *(float *)(arg0 + 0x18) + D_L20_001B2410 * r;
        func_002A9288(arg0, D_L20_001B241C);
    }
    if (((*(u16 *)(arg0 + 0x34)) ^ 1) & 1) {
        func_L20_0044AA88(arg0);
    }
}

/* WAVE 3 (2026-10-20, worker 09, unit src/levels/20_yeedil/L20_00449178-g8.c).
   The attempt above is unchanged and is still the best one; this wave
   measured exactly what the compiler does with the 16-byte copy, and where.

   THE FOLD HAPPENS IN THE CSE, NOT IN COMBINE.  Measured with `cc1 -dall'
   on a small file: the front end builds the copy as
       (set (reg/v:SI 86) (plus:SI (reg/v:SI 84) (const_int 16)))
       (set (reg:TI 88) (mem:TI (reg/v:SI 87) 0))
       (set (mem:TI (reg/v:SI 86) 0) (reg:TI 88))
   - the MEM's address IS a pseudo in the .rtl dump.  The .cse dump already
   shows (mem:TI (plus:SI (reg/v:SI 84) (const_int 16)) 0): the CSE
   replaced the pseudo with the plus.  (In this compiler the CSE runs
   BEFORE combine: toplev.c has cse_main at 3854, gcse_main at 3899,
   cse2_main at 3980 and combine_instructions at 4093.)
   The rule is find_best_addr() in gcc/cse.c: CSE_ADDRESS_COST(REG) is
   ADDRESS_COST(REG) = 1 (config/mips/mips.h:3826) and
   CSE_ADDRESS_COST(plus(reg, small const)) is mips_address_cost(plus) = 1
   (config/mips/mips.c:2646, SMALL_INT -> 1).  A TIE, broken by
   ((p->cost + 1) >> 1 > best_rtx_cost), and rtx_cost(plus) = 2 +
   rtx_cost(reg) + rtx_cost(const) = 4 against the register's 0 or 1 - so
   the plus wins.
   The reload then sees (mem:TI (plus (reg) const) 0), which IS a legitimate
   TImode address (GO_IF_LEGITIMATE_ADDRESS, the offset inside
   [-32768, 32751]), so the offset becomes the displacement.
   => The retail's shape (an addiu plus a zero-displacement lq/sq) is
      EXACTLY what the reload produces when the MEM's address is still the
      pseudo.  Only the CSE's cost tie takes it away.  A tie that went to
      the register would give the retail's bytes for every 16-byte copy in
      the game.

   WHAT MAKES THE ADDRESS SURVIVE THE CSE (all measured):  the pseudo's
   value must not be a `plus' of a register and a small constant, because
   that is the only shape with an address cost of 1.
   - a SYMBOL base: plus(symbol, const) has address cost 3, the pseudo
     survives, the reload materialises it and the access gets a ZERO
     displacement:
       lui $3,%hi(SYM+16) ; addiu $3,$3,%lo(SYM+16) ;
       lq $2,368($3) ; sq $2,0($3)
     This is the only plain-C spelling measured that reaches the retail's
     zero-displacement 16-byte access, and it needs a symbol address - the
     copies here have runtime pointer bases.
   - a MEM base (plus(mem, const), cost 4): the pseudo survives, but the
     reload keeps the offset on the loaded register, so it folds anyway:
       lw $3,104($4) ; sq $2,384($3)
   - a volatile pointer object (the value goes through memory): the pseudo
     survives and the reload materialises it:
       addiu $v0,$s0,0x10 ; sw $v0,0($sp) ; lw $v1,0($sp) ;
       lq $2,0x180($s1) ; sq $2,0($v1)
     MEASURED ON THIS FUNCTION: 184 bytes against the retail's 180 - one
     instruction OVER, because the frame grows by 16 bytes and the copy
     costs three instructions where the retail has one (the addiu).  The
     wave-3 hint ("escape B lands on a body that is already a few bytes
     short") does NOT hold here: the body is 8 bytes short and escape B
     costs 12.
   - an offset outside [-32768, 32752]: the reload materialises, but with
     the wrong immediate.

   THE COMMON FILE'S RULE "the retail materialises a 128-bit address when
   the address is used more than once" DOES NOT HOLD IN THIS COMPILER.
   Measured: a pointer used twice in one block (two 16-byte copies to it
   and from it) and a pointer live across a call are all still folded:
       sq $2,16($4) ; sq $2,272($4) ; sq $2,16($18)
   The retail's shape in this function is the two addiu's of the copy, and
   the difference in the float registers below follows from it.

   THE FLOAT REGISTER ORDER IS A SCHEDULING DIFFERENCE, NOT A SOURCE
   SHAPE: this cc1 materialises the literal 0x00029088 into $f0 and loads
   the $gp float into $f13 (mul.s $f13,$f13,$f0), the retail loads the $gp
   float into $f0 and materialises the literal into $f13 (mul.s
   $f13,$f0,$f13).  -fno-schedule-insns moves the materialisation, but it
   BREAKS the two bodies already matched in the unit (func_L20_00449AE8:
   "instruction at +0x60 differs", func_L20_0044AA88: "instruction at
   +0xc differs"), so it cannot go on this file. */
