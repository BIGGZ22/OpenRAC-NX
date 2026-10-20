#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef int s32;
typedef float f32;

/*
 * func_L04_0046D6A8 -- not attempted: the third place the opcode-0x1C word 0x700014A9
 * appears in this lot (with func_L16_00413700 and func_L23_0042AD10).
 *
 * retail 0x0046D6A8, 192 bytes, level 4 (04_barlow), two places. Instruction map
 * (0x1C-group words marked *; rabbitizer prints them as `.word # INVALID`):
 *   0x0046D6A8  addiu sp,sp,-0x50 ; v1 = 0x00189EA0 (lui 0x19 + addiu -0x6160)
 *               sd s0,0x20(sp) ; sd s1,0x28(sp) ; s0 = a0 ; sd ra,0x30(sp) ; swc1 f20,0x40(sp)
 *   0x0046D6C8  a0 = *(void **)(s0+0x70)
 *   0x0046D6CC  *0x78620000 (lq $v0,0x0($v1)) ; *0x7FA20000 (sq $v0,0x0($sp))
 *   0x0046D6D4  f1 = *(f32 *)(a0+0x90)
 *   0x0046D6D8  *0x700014A9      (opcode 0x1C, rs=rt=0, rd=$v0, sa=0x12, funct 0x29)
 *   0x0046D6DC  f0 = *(f32 *)(sp+0x8) ; a1 = sp ; f20 = 1.0f ; a2 = s0+0x30
 *   0x0046D6F0  f0 = f0 + f1 ; *0x7FA20010 (sq $v0,0x10($sp)) ; f20 -> sp+0x18, sp+0x1C
 *   0x0046D6FC  a0 = s0 ; s1 = s0+0x10 ; jal func_00282CF0 ; f0 -> sp+0x8
 *   0x0046D710  f12 = f20 ; a0 = s0 ; jal func_00282F20 ; a1 = s0
 *   0x0046D720  a2 = sp+0x10 ; a0 = s1 ; jal func_00282DD8 ; a1 = s0
 *   0x0046D730  f12 = f20 ; a0 = s1 ; jal func_00282F20 ; a1 = s1
 *   0x0046D740  a1 = s1 ; a2 = s0 ; jal func_00282DD8 ; a0 = s0+0x20
 *   0x0046D750  epilogue (ld s0/s1/ra, lwc1 f20, jr ra, addiu sp,sp,0x50)
 *
 * In all three functions the 0x1C-group word sits inside a 16-byte lq/sq stack copy
 * sequence and, unlike its neighbours, has no live general register: in this one and
 * in func_L16_00413700 the quadword in $v0 is still needed afterwards, so the word
 * cannot be writing $v0, and it never reads or writes a value the body uses. That is
 * the shape of a compiler-emitted filler/hint (the same word is 0x70001CA9 in
 * func_L01_002F1FD0's row), not of a data operation. The rest of the body - build a
 * {x, y, 1.0f, 1.0f} vector on the stack from a static 0x00189EA0 block plus the
 * sub-object's 0x90 scalar, then run the three transform calls - is spelled out above.
 * Identifying the word is the lead's call; ASM_FUNC candidate.
 */

INCLUDE_ASM("asm/levels", func_L04_0046D6A8);
