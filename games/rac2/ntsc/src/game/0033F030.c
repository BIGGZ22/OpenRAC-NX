#include "include_asm.h"

extern void func_0033AF48_2();

extern void func_002DAF68_1();

extern void func_00336A78_5();

extern void func_00336828();

extern void func_00336320_4();

extern void func_0033ED88();

typedef unsigned char u8;

extern void func_L00_0042BAD0_6(u8 *);

extern void func_L00_0042BFF8_5(u8 *);

extern void func_L00_0042C290_6(u8 *);

extern void func_00347EA0_1(u8 *);

typedef int s32;

/* Forwards one object pointer, eight pointer-plus-constant arguments and a null to ten calls in a fixed order. */
void func_0033F030(char *p)
{
    func_0033AF48_2(p + 8);
    func_002DAF68_1(0);
    func_00336A78_5(p + 744);
    func_00336A78_5(p + 816);
    func_00336A78_5(p + 888);
    func_00336828(p + 684);
    func_00336320_4(p + 960);
    func_00336A78_5(p + 1400);
    func_00336A78_5(p + 1328);
    func_0033ED88(p);
}

INCLUDE_ASM("asm/game", func_0033F0A0);

INCLUDE_ASM("asm/game", func_0033F210);

INCLUDE_ASM("asm/game", func_0033F218);

INCLUDE_ASM("asm/game", func_0033FB98);

INCLUDE_ASM("asm/game", func_00340250);

INCLUDE_ASM("asm/game", func_00340498);

INCLUDE_ASM("asm/game", func_00340638);

INCLUDE_ASM("asm/game", func_003407F8);

INCLUDE_ASM("asm/game", func_003409C8);

INCLUDE_ASM("asm/game", func_00340B70);

INCLUDE_ASM("asm/game", func_00340D18);

INCLUDE_ASM("asm/game", func_00340D68);

INCLUDE_ASM("asm/game", func_00341030);

INCLUDE_ASM("asm/game", func_003411B0);

/* initialises eight in-place sub-records of one object by four routines, then returns the object pointer */
u8 *func_003411C0(u8 *a0) {
    func_L00_0042BAD0_6(a0);
    func_L00_0042BAD0_6(a0 + 76);
    func_L00_0042BAD0_6(a0 + 152);
    func_L00_0042BAD0_6(a0 + 228);
    func_L00_0042BAD0_6(a0 + 304);
    func_L00_0042BFF8_5(a0 + 380);
    func_L00_0042C290_6(a0 + 464);
    func_00347EA0_1(a0 + 552);
    return a0;
}

INCLUDE_ASM("asm/game", func_00341220);

/* stores three consecutive words; byte-identical across all 27 levels. */
void func_00341540(u8 *a0, s32 a1, s32 a2, s32 a3) { *(s32 *)(a0 + 0x2F0) = a1; *(s32 *)(a0 + 0x2F4) = a2; *(s32 *)(a0 + 0x2F8) = a3; }

/* Measured target: byte-identical across all 27 levels. */
void func_00341550(u8 *a0, s32 a1) { *(s32 *)(a0 + 0x220) = a1; }

INCLUDE_ASM("asm/game", func_00341558);

INCLUDE_ASM("asm/game", func_003415B8);

INCLUDE_ASM("asm/game", func_00341610);

INCLUDE_ASM("asm/game", func_00341760);

void func_003417C8(void)
{
}

INCLUDE_ASM("asm/game", func_003417D0);
