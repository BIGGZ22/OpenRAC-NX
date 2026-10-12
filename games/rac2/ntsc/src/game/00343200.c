#include "include_asm.h"

typedef unsigned char u8;

extern void func_L00_0042BAD0_3(u8 *);

extern void func_L00_0042BFF8_2(u8 *);

extern void func_L00_0042C290_3(u8 *);

extern void func_L00_004380E0_1(u8 *);

/* Measured partial C views; original types and object ownership are unknown. */
typedef unsigned char QwenRecovery_a2544d80ec96_AbiByte22;

extern void func_00342F70(QwenRecovery_a2544d80ec96_AbiByte22 *owner, const QwenRecovery_a2544d80ec96_AbiByte22 *table);

extern int func_00342BC0(unsigned char *owner);

extern void func_00343058(unsigned char *owner);

typedef int s32;

typedef unsigned int u32;

/* initialises eight in-place sub-records of one object by four routines, then returns the object pointer */
u8 *func_00343200(u8 *a0) {
    func_L00_0042BAD0_3(a0);
    func_L00_0042BAD0_3(a0 + 76);
    func_L00_0042BAD0_3(a0 + 152);
    func_L00_0042BFF8_2(a0 + 228);
    func_L00_0042C290_3(a0 + 432);
    func_L00_0042C290_3(a0 + 504);
    func_L00_0042C290_3(a0 + 576);
    func_L00_004380E0_1(a0 + 664);
    return a0;
}

INCLUDE_ASM("asm/game", func_00343260);

void func_00343538(QwenRecovery_a2544d80ec96_AbiByte22 *p1, const QwenRecovery_a2544d80ec96_AbiByte22 *table)
{
    func_00342F70(p1 + 0x298, table);
}

INCLUDE_ASM("asm/game", func_00343558);

int func_00343560(unsigned char *owner)
{
    return func_00342BC0(owner + 0x298);
}

void func_00343580(unsigned char *owner)
{
    func_00343058(owner + 0x298);
}

/* Measured target: byte-identical across all 27 levels. */
void func_003435A0(u8 *a0, s32 a1) { *(s32 *)(a0 + 0x330) = a1; }

/* Measured target: byte-identical across all 27 levels. */
s32 func_003435A8(u8 *a0) { return *(s32 *)(a0 + 0x330); }

void func_003435B0(u8 *a0, u32 a1) {
    if (a1 < 4) *(u32 *)(a0 + 0x294) = a1;
    *(s32 *)(a0 + 0x1a0) = 0;
}

INCLUDE_ASM("asm/game", func_003435C8);

INCLUDE_ASM("asm/game", func_00343638);
