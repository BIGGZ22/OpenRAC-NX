#include "include_asm.h"

extern void func_00336320_6(char *);

extern void func_00336A78_7(char *);

extern void func_00341A48(char *);

extern void func_00341A68_1(char *);

extern void func_00341A88(char *);

typedef int s32;

typedef unsigned char u8;

extern void func_00348068(unsigned char *owner, float *records);

void func_00348138(int);

void func_00348138(int param_1);

/* Reads a word at +544 of the object; returns when it is zero, otherwise calls a repeated callee four times with the object and the objects at +152, +76 and +228, then one callee each with +464, the object, the object and the object. */
void func_00341C20(char *a0)
{
    if (*(int *)(a0 + 544) == 0)
        return;
    func_00336320_6(a0);
    func_00336320_6(a0 + 152);
    func_00336320_6(a0 + 76);
    func_00336320_6(a0 + 228);
    func_00336A78_7(a0 + 464);
    func_00341A48(a0);
    func_00341A68_1(a0);
    func_00341A88(a0);
}

INCLUDE_ASM("asm/game", func_00341C90);

INCLUDE_ASM("asm/game", func_00341CD0);

/* Measured target: byte-identical across all 27 levels. */
void func_00341CD8(u8 *a0, s32 a1) { *(s32 *)(a0 + 0x8) = a1; }

INCLUDE_ASM("asm/game", func_00341CE0);

INCLUDE_ASM("asm/game", func_00341E50);

INCLUDE_ASM("asm/game", func_00341E58);

INCLUDE_ASM("asm/game", func_00341E90);

INCLUDE_ASM("asm/game", func_00341EB0);

INCLUDE_ASM("asm/game", func_00341EE8);

INCLUDE_ASM("asm/game", func_00341F38);

INCLUDE_ASM("asm/game", func_003420C8);

INCLUDE_ASM("asm/game", func_00342118);

void func_00342380(unsigned char *owner, float *records)
{
    *(float **)(owner + 0x250) = records;
    func_00348068(owner + 0x188, records);
}

/* Measured target: byte-identical across all 27 levels. */
void func_003423A8(u8 *a0, s32 a1) { *(s32 *)(a0 + 0x170) = a1; }

/* multiplies the index by 20 and adds it to a base; byte-identical across all 27 levels. */
s32 func_003423B0(u8 *a0) { return *(s32 *)(a0 + 0x250) + *(s32 *)(a0 + 0x16C) * 20; }

INCLUDE_ASM("asm/game", func_003423C8);

void func_00342420(void)
{
}

INCLUDE_ASM("asm/game", func_00342428);

INCLUDE_ASM("asm/game", func_003424C0);

INCLUDE_ASM("asm/game", func_003425B8);

void func_00342648(int param_1)
{
  func_00348138(param_1 + 0x188);
  return;
}
