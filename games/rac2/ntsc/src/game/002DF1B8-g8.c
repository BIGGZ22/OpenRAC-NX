#include "include_asm.h"

/* -G8 sibling for src/game/002DF1B8.c: the two argument words go out through
   $gp, so this body needs the small-data addressing mode. */

extern void func_002B49E0(int, int, int, int, int);

extern void func_00284E18(int, int, int);

extern int D_001ABD80 __attribute__((sda));
extern int D_001ABD84 __attribute__((sda));

extern int D_001B1608[] __attribute__((sda));
extern char *D_001B1B5C;
extern char *D_001B1B64;

extern char *D_001B22A8;
extern int D_001ABD88[2] __attribute__((sda));
extern char D_00261D10[];
extern void func_002E0C40(int, int);

/* func_002DF380 is here for the compiler, not for $gp: the SDK compiler of
   src/game/002DF1B8.c does not rotate the loop the way retail does. The first
   call returns a word the original keeps in $v0, which is what puts the base
   pointer in $v1. */
extern int func_002B4B70(int, int);

/* The globals this function reads are above the resident data: their addresses
   differ from program to program, so they have no address in their names. */
extern int ref_L00_002DF380_00[];
extern char *ref_L00_002DF380_01;
extern char *ref_L00_002DF380_02;

void func_002DF380(void)
{
    char *start;
    char *end;
    char *p;

    func_002B4B70(0, 0);
    func_00284E18(ref_L00_002DF380_00[0x8F], ref_L00_002DF380_00[0x90], ref_L00_002DF380_00[0x91]);
    start = ref_L00_002DF380_02;
    end = ref_L00_002DF380_01;
    for (p = start; p < end; p += 0x100) {
        *(unsigned short *)(p + 0x34) &= 0xFF7F;
    }
}

void func_002DF7A8(int a0, int a1, int a2)
{
    *(int *)(*(char *volatile *)&D_001B22A8 + 0x0) = 0x30000003;
    *(int *)(*(char *volatile *)&D_001B22A8 + 0x4) = (int)(D_00261D10 + a2 * 0x30);
    *(int *)(*(char *volatile *)&D_001B22A8 + 0x8) = 0x13000000;
    *(int *)(*(char *volatile *)&D_001B22A8 + 0xC) = 0x50000003;
    D_001B22A8 = *(char *volatile *)&D_001B22A8 + 0x10;
    func_002E0C40(a0, D_001ABD88[a1]);
}

void func_002DF2E0(int a0, int a1)
{
    char *start;
    char *end;
    char *p;

    func_002B49E0(7, 1, 0, 0, 0);
    D_001ABD84 = a1;
    D_001ABD80 = a0;
    D_001B1608[0] = 0;
    func_00284E18(0, 0, 0);
    start = D_001B1B5C;
    end = D_001B1B64;
    for (p = start; p < end; p += 0x100) {
        *(unsigned short *)(p + 0x34) |= 0x80;
    }
}

extern char *D_001B20B4;
extern int D_001B20BC __attribute__((section(".sdata")));
extern void *D_001A72D0 __attribute__((sda));
extern char D_001ABD90[];
extern int func_002E2988(void *);
extern void func_002FC748(void);
extern void func_0026FEB8(const char *, ...);

void func_002E0F98(void)
{
    char *other = *(char *volatile *)&D_001B20B4;
    char *saved = *(char *volatile *)&D_001B22A8;
    char *next;
    int size;
    D_001B22A8 = saved + 16;
    *(unsigned int *)other = 0x20000000;
    *(int *)(*(char *volatile *)&D_001B20B4 + 4) = (int)*(char *volatile *)&D_001B22A8;
    *(int *)(*(char *volatile *)&D_001B20B4 + 8) = 0;
    *(int *)(*(char *volatile *)&D_001B20B4 + 12) = 0;
    size = func_002E2988(D_001A72D0);
    func_002FC748();
    if (size > 0x400000)
        func_0026FEB8(D_001ABD90);
    if (*(volatile int *)&D_001B20BC < size)
        D_001B20BC = size;
    *(unsigned int *)(*(char *volatile *)&D_001B22A8) = 0x20000000;
    next = *(char *volatile *)&D_001B20B4 + 16;
    *(int *)(*(char *volatile *)&D_001B22A8 + 4) = (int)next;
    *(int *)(*(char *volatile *)&D_001B22A8 + 8) = 0;
    *(int *)(*(char *volatile *)&D_001B22A8 + 12) = 0;
    *(char *volatile *)&D_001B22A8 = *(char *volatile *)&D_001B22A8 + 16;
    *(unsigned int *)saved = 0x20000000;
    *(int *)(saved + 4) = (int)*(char *volatile *)&D_001B22A8;
    *(int *)(saved + 8) = 0;
    *(int *)(saved + 12) = 0;
}
