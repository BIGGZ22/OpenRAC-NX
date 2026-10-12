#include "include_asm.h"

extern void func_00338810(char *a0);

extern void func_L00_0042E388_1(char *a0);

extern void func_L00_0042E138_1(char *a0);

extern void func_L00_0042BAD0_10(void *p);

extern void func_L00_0042C290_10(void *p);

extern void func_00336320_1(char *a);

extern void func_00336A78_1(char *a);

/* Returns early when a float field reached through a two-level pointer chain is exactly zero; otherwise dispatches on an integer state (0 or 2 selects one of two callees) and then always calls a third callee with the same object pointer. */
void func_00338A60(char *a0)
{
    if (*(float *)(*(int *)(a0 + 4)) == 0.0f)
        return;

    switch (*(int *)(a0 + 456))
    {
    case 0:
        func_00338810(a0);
        break;
    case 2:
        func_L00_0042E388_1(a0);
        break;
    }

    func_L00_0042E138_1(a0);
}

INCLUDE_ASM("asm/game", func_00338AD8);

void func_00338F50(void)
{
}

INCLUDE_ASM("asm/game", func_00338F58);

/* Initialises eight sub-objects at byte offsets 8, 84, 160, 236 (step 76) and 312, 384, 456, 528 (step 72) of the argument with two different helpers and returns the argument. */
void *func_00339008(char *a0)
{
    func_L00_0042BAD0_10(a0 + 8);
    func_L00_0042BAD0_10(a0 + 84);
    func_L00_0042BAD0_10(a0 + 160);
    func_L00_0042BAD0_10(a0 + 236);
    func_L00_0042C290_10(a0 + 312);
    func_L00_0042C290_10(a0 + 384);
    func_L00_0042C290_10(a0 + 456);
    func_L00_0042C290_10(a0 + 528);
    return a0;
}

INCLUDE_ASM("asm/game", func_00339070);

INCLUDE_ASM("asm/game", func_00339078);

INCLUDE_ASM("asm/game", func_00339320);

/* early-returns when a 32-bit field is zero, otherwise calls one helper four times and another three times with fixed byte offsets off the argument pointer. */
void func_00339590(char *a0)
{
    if (*(int *)(a0 + 664) == 0)
        return;
    func_00336320_1(a0 + 8);
    func_00336320_1(a0 + 84);
    func_00336320_1(a0 + 160);
    func_00336320_1(a0 + 236);
    func_00336A78_1(a0 + 312);
    func_00336A78_1(a0 + 384);
    func_00336A78_1(a0 + 456);
}

char *func_003395F8(char *object)
{
    extern void func_0033AB50(void *);
    extern void *func_003361A8(void *);
    func_0033AB50(object + 8);
    func_003361A8(object + 0x2A8);
    return object;
}
