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

extern void func_00335FB0(void *);
extern char D_001ADA18[] __attribute__((sda));

char *func_00336968(char *object) {
    func_00335FB0(object);
    *(char **)(object + 0x30) = D_001ADA18;
    return object;
}

INCLUDE_ASM("asm/game", func_003369A0);

extern void func_00335FC8(void *);
extern unsigned char D_00263B50[];

void func_003369A8(char *self)
{
    func_00335FC8(self);
    *(char **)(self + 0x34) = (char *)D_00263B50;
    *(long long *)(self + 0x38) = 1;
    *(int *)(self + 0x40) = 0;
    **(float **)(self + 4) = 1.0f;
    (*(float **)(self + 4))[1] = 1.0f;
    *(int *)(self + 0x44) = 1;
}

extern char D_001ADA18[] __attribute__((sda));
extern void func_003360F8(void *);

void func_00336A08(char *object) {
    *(char **)(object + 0x30) = D_001ADA18;
    func_003360F8(object);
}

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

extern void func_0027F1D8(void *a0, s32 a1, void *a2, f32 a3);

void func_00336A40(u8 *self)
{
    func_0027F1D8(*(void **)(self + 0x40), -1, *(void **)(self + 0x34), *(f32 *)*(void **)(self + 4));
}

INCLUDE_ASM("asm/game", func_00336A70);
