#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef int s32;
typedef float f32;

/*
 * func_L23_0042AD10 -- not attempted: an opcode-0x1C word in the body.
 *
 * retail 0x0042AD10, 128 bytes, level 23 (23_damosel_orbit). Instruction map
 * (0x1C-group words marked *; rabbitizer prints them as `.word # INVALID`):
 *   0x0042AD10  addiu sp,sp,-0x20 / sd ra,0x10(sp) / a1 = a0
 *   0x0042AD1C  lbu v0,0x20(a1) / bnez v0,epilogue ; a2 = *(void **)(a1+0x68)
 *   0x0042AD28  v0 = 0x50 ; v1 = 0xFF ; sh v0,0x32(a1) ; a0 = a1+0x10 ; sb v1,0x30(a1)
 *   0x0042AD3C  *0x700014A9        (opcode 0x1C, rs=rt=0, rd=$v0, sa=0x12, funct 0x29)
 *   0x0042AD40  sw zero,0x4(a2) ; sw zero,0x0(a2)
 *   0x0042AD48  f1 = 500.0f (0x43FA0000) ; f0 = 584.0f (0x44124000)
 *   0x0042AD5C  *0x7FA20000 (sq $v0,0x0($sp)) ; swc1 f1,0x4(sp) ; swc1 f0,0x8(sp)
 *               ; swc1 f1,0x0(sp) ; *0x7BA20000 (lq $v0,0x0($sp)) ; *0x7C820000 (sq $v0,0x0($a0))
 *   0x0042AD74  a0 = a1 ; a2 = 0 ; jal func_002A7750 ; a1 = 1
 *   0x0042AD84  ld ra,0x10(sp) / jr ra / addiu sp,sp,0x20
 *
 * The body is readable (arm a 0x50/0xFF state, clear two words of the sub-object,
 * then copy a {500,584,500} vector over a1+0x10 and hand the object to func_002A7750
 * with mode 1), but the same unidentified 0x700014A9 word that blocks
 * func_L16_00413700 sits at 0x0042AD3C, and the 16-byte stack copy is emitted as
 * sq/lq around three swc1 stores whose register is live before them. Both need the
 * 0x1C-group instruction identified (README: func_L01_002F1FD0, func_L00_00387BB0).
 * No C spelling is known for that word: ASM_FUNC candidate.
 */

INCLUDE_ASM("asm/levels", func_L23_0042AD10);
