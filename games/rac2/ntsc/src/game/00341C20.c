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

char *func_00341C90(char *p) {
    extern void func_00347EA0(void *);
    extern void func_003361A8(char *);
    func_00347EA0(p + 0x10);
    func_003361A8(p + 0xDC);
    func_003361A8(p + 0x128);
    return p;
}

INCLUDE_ASM("asm/game", func_00341CD0);

/* Measured target: byte-identical across all 27 levels. */
void func_00341CD8(u8 *a0, s32 a1) { *(s32 *)(a0 + 0x8) = a1; }

INCLUDE_ASM("asm/game", func_00341CE0);

INCLUDE_ASM("asm/game", func_00341E50);

INCLUDE_ASM("asm/game", func_00341E58);

extern void func_00348128(void *);

void func_00341E90(char *object) {
    func_00348128(object + 0x10);
}

void func_00341EB0(unsigned char *owner, float *records)
{
    func_00348068(owner + 0x10, records);
    *(float **)(owner + 0xD8) = records;
}

INCLUDE_ASM("asm/game", func_00341EE8);

INCLUDE_ASM("asm/game", func_00341F38);

char *func_003420C8(char *object) {
    extern void func_003361A8(char *);
    extern void func_003366D0(char *);
    extern void func_00347EA0(void *);
    func_003361A8(object);
    func_003361A8(object + 0x4C);
    func_003361A8(object + 0x98);
    func_003361A8(object + 0xE4);
    func_003366D0(object + 0x130);
    func_00347EA0(object + 0x188);
    return object;
}

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
