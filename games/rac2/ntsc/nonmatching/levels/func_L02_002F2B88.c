/* 0 of 1 places match - COP2/VU0 macro-mode function (out of C reach).
   Level 2 (02_maktar_nebula), 0x002F2B88, 104 bytes. Decoded shape:
     ldc2 $4,0x00($a1) ; ldc2 $5,0x10($a1) ; ldc2 $6,0x20($a1)   (3 columns of m1)
     ldc2 $1,0x00($a2) ; ldc2 $2,0x10($a2) ; ldc2 $3,0x20($a2)   (3 columns of m2)
     6 x COP2 macro ops (opcode 0x12, e.g. 0x4BE121BC = vmadd/vmul on $at,$v0,$v1)
     lw $at,0xC($a0) ; lw $v0,0x1C($a0) ; lw $v1,0x2C($a0)       (translation column)
     3 more COP2 ops
     sdc2 $1,0x00($a0) ; sdc2 $2,0x10($a0) ; sdc2 $3,0x20($a0)
     sw $at,0xC($a0) ; sw $v0,0x1C($a0) ; sw $v1,0x2C($a0)
     jr $ra ; nop
   i.e. a 4x4 matrix multiply dst = m1 * m2 with the translation column moved in
   and out through $at/$v0/$v1. rabbitizer prints the COP2 words as `.word #
   INVALID`, they are live VU0 instructions. Needs ASM_FUNC (lead decision for
   COP2); no C form reaches COP2 macro mode with this compiler. */

#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L02_002F2B88);
