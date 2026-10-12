#include "include_asm.h"

extern void func_0011AC60(int value);

extern void func_00350808_1(int value);

extern void func_00350798(int value);

extern void func_0011AC40(int value);

/* snapshots five hardware registers into a structure, spins while one register's nibble stays set, snapshots five more and returns 1 */
int func_00350DC8(int *p)
{
    func_0011AC60(p[16]);
    p[17] = 0;
    func_00350808_1(5);
    p[7] = *(volatile int *)0x1000B410;
    p[8] = *(volatile int *)0x1000B430;
    p[9] = *(volatile int *)0x1000B420;
    p[10] = *(volatile int *)0x1000B400;
    if (*(volatile int *)0x10002010 & 0xF0)
        while (*(volatile int *)0x10002010 & 0xF0)
            ;
    func_00350798(0);
    p[11] = *(volatile int *)0x1000B010;
    p[12] = *(volatile int *)0x1000B020;
    p[13] = *(volatile int *)0x1000B000;
    p[14] = *(volatile int *)0x10002020;
    p[15] = *(volatile int *)0x10002010;
    func_0011AC40(p[16]);
    return 1;
}

INCLUDE_ASM("asm/game", func_00350ED8);

INCLUDE_ASM("asm/game", func_00351210);
