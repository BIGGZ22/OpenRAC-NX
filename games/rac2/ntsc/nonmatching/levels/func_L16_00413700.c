#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef int s32;
typedef float f32;

/*
 * func_L16_00413700 -- not attempted: an opcode-0x1C word in the body.
 *
 * retail 0x00413700, 136 bytes, level 16 (16_snivelak). Instruction map
 * (0x1C-group words marked *; rabbitizer prints them as `.word # INVALID`):
 *   0x00413700  addiu sp,sp,-0x30 ; sd s1,0x18(sp) ; sd ra,0x20(sp) ; s1 = a0 ; sd s0,0x10(sp)
 *   0x00413714  s0 = *(void **)(s1+0x68)
 *   0x00413718  *0x78A20000 (lq $v0,0x0($a1)) ; *0x7E020000 (sq $v0,0x0($s0))
 *   0x00413720  swc1 $f12, 0x34($s0)
 *   0x00413724  *0x700014A9      (opcode 0x1C, rs=rt=0, rd=$v0, sa=0x12, funct 0x29)
 *   0x00413728  f0 = 1.0f (0x3F800000) ; s0 += 0x20
 *   0x00413734  *0x7FA20000 (sq $v0,0x0($sp)) ; a2 = a1 ; a1 = sp ; swc1 f0,0x8(sp)
 *   0x00413744  jal func_00282DD8 ; a0 = s0
 *   0x0041374C  a0 = s0 ; f12 = 0x3D6E4BB0 (0.0581f)
 *   0x0041375C  jal func_002ACE78 ; a1 = a0 (= s0)
 *   0x00413764  a0 = s1 ; a1 = 2 ; jal func_002A7750 ; a2 = -1
 *   0x00413774  ld s0,0x10(sp) / ld s1,0x18(sp) / ld ra,0x20(sp) / jr ra / addiu sp,sp,0x30
 *
 * Read: the sub-object gets a 16-byte copy of the vector at a1, a scalar at +0x34,
 * then a {x,y,1.0f,w} copy is passed with the vector to func_00282DD8 and the timers
 * at +0x20 are handed to func_002ACE78 with 0.0581f before returning to state 2.
 * The same 0x700014A9 word as func_L23_0042AD10 sits at 0x00413724 and blocks
 * byte-exactness on its own: no C spelling is known for the 0x1C-group instruction
 * (README: func_L01_002F1FD0, func_L00_00387BB0).
 * The lq/sq pairs themselves are not the wall - C struct copies reach them
 * (README: func_L00_0037FA90 is 83/84 with them). ASM_FUNC candidate.
 */

INCLUDE_ASM("asm/levels", func_L16_00413700);
