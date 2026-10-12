#include "include_asm.h"

typedef int s32;

typedef unsigned char u8;

typedef float f32;

void func_00336920(u8 *a0,s32 a1,s32 a2) {
 **(f32 **)(a0+0x34)=(f32)a1;
 (*(f32 **)(a0+0x34))[1]=(f32)a2;
}

/* reads a float through double indirection and returns its integer value; byte-identical across all 27 levels. */
s32 func_00336950(u8 *a0) { return (s32)*(f32 *)(*(u8 **)(a0 + 0x34)); }

INCLUDE_ASM("asm/game", func_00336968);

INCLUDE_ASM("asm/game", func_003369A0);

INCLUDE_ASM("asm/game", func_003369A8);

INCLUDE_ASM("asm/game", func_00336A08);

void func_00336A30(int param_1, unsigned int param_2)
{
  *(unsigned int*)(param_1 + 0x44) = param_2;
  return;
}

void func_00336A38(int param_1, unsigned int param_2)
{
  *(unsigned int *)(param_1 + 0x40) = param_2;
  return;
}

INCLUDE_ASM("asm/game", func_00336A40);

INCLUDE_ASM("asm/game", func_00336A70);
