#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef int s32;

/*
 * func_L19_002A88C8 -- not attempted: a `.word 0x00651818` multiply-shape word whose
 * result nothing reads.
 *
 * retail 0x002A88C8, 148 bytes, level 19 (19_grelbin). Instruction map:
 *   0x002A88C8  addiu sp,sp,-0x10 ; lui v0,0x19 ; sd ra,0x0(sp)
 *   0x002A88D4  a1 = 0x00189E20 (lui 0x19 + addiu -0x61E0) ; a2 = 0x00190000
 *   0x002A88DC  v1 = *(u8 *)(a1+0x22B4)      (0x0018C0D4)
 *   0x002A88E0  v0 = 4 ; beq v1,v0,epilogue ; v0 = 0xD
 *   0x002A88EC  beqz a0, 0x002A8940 ; v1 = a2-0x61E0 (= a1)
 *   0x002A88F4  v0 = *(u8 *)(a1+0x22BA)      (0x0018C0DA)
 *   0x002A88F8  beqz v0, 0x002A8944 ; v0 = 0x51
 *   0x002A8900  v0 = *(s16 *)(a1+0x24EC)     (0x0018C30C)
 *   0x002A8904  bnel v0,$zero,0x002A8944 ; v0 = 0x51
 *   0x002A890C  jal func_L00_002ADEA0 (level 19: 0x002B0CA8) ; a0 = 0
 *   0x002A8914  v1 = 0x00139568 (lui 0x14 + addiu -0x6A98) ; a1 = 0xE0 ; v0 += v1
 *   0x002A892C  a0 = 0x00265870 (lui 0x26 + addiu 0x5870) ; v1 = *(u8 *)v0
 *   0x002A8930  .word 0x00651818   <-- SPECIAL funct 0x18 (mult) rs=$v1 rt=$a1 rd=$v1
 *   0x002A8934  v1 += a0 ; b 0x002A8950 ; v0 = *(s32 *)(v1+0x28)
 *   0x002A8940  v0 = 0x51
 *   0x002A8944  a0 = *(s32 *)(v1+0x24CC) ; a0 ^= 1 ; movn v0,zero,a0
 *   0x002A8950  ld ra,0x0(sp) ; jr ra ; addiu sp,sp,0x10
 *
 * Read: func_L00_002ADEA0(0) returns an index into the byte table at 0x00139568; the
 * byte then selects a record whose word at +0x28 is returned (the 0x51/0xD defaults
 * come from the gate at 0x0018C0DA and the halfword at 0x0018C30C). The whole body is
 * a table lookup, so it is writable in C, but the multiply at 0x002A8930 computes
 * $v1 * 0xE0 into a register that the very next delay slot overwrites: the product is
 * dead in the retail, and the byte/word index arithmetic (`base + byte`, no scaling)
 * does not correspond to a scaled record stride either. `mult` with a destination
 * register and a dead result is not something this cc1 emits, so the word needs
 * identifying first (the same 3-operand MULT shape as the R5900's MULT1 note).
 * ASM_FUNC candidate.
 */

INCLUDE_ASM("asm/levels", func_L19_002A88C8);
