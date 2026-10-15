#include "include_asm.h"

typedef int s32;

typedef unsigned char u8;

typedef unsigned int u32;

extern int func_00342BC0(unsigned char *owner);

typedef float f32;

extern void func_00343058(unsigned char *owner);

/* Target: func_003427B0 — identity function returning its first argument */
/* Disassembly: jr ra; move v0,a0 (MIPS delay slot) */
/* Compiled with: -O2 -G0 -ffunction-sections */

unsigned long func_003427B0(unsigned long param_1)
{
  return param_1;
}

INCLUDE_ASM("asm/game", func_003427B8);

INCLUDE_ASM("asm/game", func_003427C0);

extern void *func_00336E40(void *);
extern void *func_00336DA8(void *, void *);
extern void *func_00115484(void *, int, int);
void func_003428E0(char *object, void *argument)
{
    *(void **)(object + 0x8C) = argument;
    if (argument) {
        int *allocation = func_00336DA8((void *)16, func_00336E40(argument));
        *(int **)object = allocation;
        allocation[1] = 0;
        allocation[2] = 0;
        allocation[3] = 0;
        allocation[0] = 0;
    }
    func_00115484(object + 0x28, 0, 0x64);
    *(int *)(object + 0x10) = 0;
    *(int *)(object + 0x20) = 0;
    *(int *)(object + 0x24) = 0;
    *(int *)(object + 0x14) = 0;
    *(float *)(object + 4) = 16.0f;
    *(int *)(object + 8) = 0;
    *(int *)(object + 0x18) = 0;
    *(int *)(object + 0x1C) = 0;
    *(int *)(object + 0x90) = 0;
    *(float *)(object + 0xC) = 16.0f;
    *(int *)(object + 0x94) = 0;
}

INCLUDE_ASM("asm/game", func_00342978);

s32 func_00342BC0(u8 *a0) {
 if(*(s32 *)(a0+0x10)!=0) return *(s32 *)((u32)a0+(*(s32 *)(a0+0x14)<<2)+0x28);
 return 0;
}

INCLUDE_ASM("asm/game", func_00342BE8);

INCLUDE_ASM("asm/game", func_00342F70);

INCLUDE_ASM("asm/game", func_00343020);

/* lot11 -- measured bodies with sizes recorded in the catalogue. */


void func_00343028(u8 *a0, f32 a1, f32 a2) { *(f32 *)(a0 + 0x8) = a1; *(f32 *)(a0 + 0xc) = a2; }

/* Measured target: byte-identical across all 27 levels. */
void func_00343038(u8 *a0, f32 a1) { *(f32 *)(a0 + 0x4) = a1; }

void func_00343040(float **position, float x, float y) {
    (*position)[0] = x;
    (*position)[1] = y;
}

/* Measured target: byte-identical across all 27 levels. */
void func_00343058(u8 *a0) { *(s32 *)(a0 + 0x90) = 0; }

/* Measured target: byte-identical across all 27 levels. */
s32 func_00343060(u8 *a0) { return *(s32 *)(a0 + 0x10); }
