#include "common.h"
#include "structs.h"

extern int r1_func_0011D960(void);

extern void r1_func_0011D9A8(void);

extern void r1_func_00128560();

extern void r1_func_0012BC78();

extern long r1_func_0012AAC8();

extern long r1_func_0012AAC8(void *, int);

/* rac1-decomp src/core/0012A2F0.c: func_0012A2F0 */
/* _doCSC (libmpeg). Stack-size probe #2: a 32-byte local (two 16-byte
   compiler stack slots) instead of one, since a single int/4-int array
   both land in one slot here (frame still 0x10 short of retail's 0x70,
   see c1-c4). */
void func_0012E110(void *arg0, unsigned int arg1, int arg2) {
    char *p = (char *)arg0;
    int r;
    int event[8];

    while (*(volatile int *)0x10002010 < 0) {
    }
    r = r1_func_0011D960();
    *(volatile unsigned int *)0x1000B010 = arg1 & 0x0FFFFFFF;
    *(volatile unsigned int *)0x1000B020 = (unsigned int)arg2 << 6;
    *(volatile unsigned int *)0x1000B000 = 0x100;
    if (r != 0) {
        r1_func_0011D9A8();
    }
    r1_func_00128560(arg0, arg2 | 0x70000000);
    event[0] = 4;
    r1_func_0012BC78(*(int *)(p + 0x858), event);
    while (((*(volatile unsigned int *)0x1000B000 >> 8) & 1) != 0) {
    }
    while (*(volatile int *)0x10002010 < 0) {
    }
}

ASM_FUNC("asm/core", func_0012E238);

INCLUDE_ASM("asm/core", func_0012E378);

ASM_FUNC("asm/core", func_0012E538);

INCLUDE_ASM("asm/core", func_0012E608);

/* rac1-decomp src/core/0012A2F0.c: func_0012AA70 */
/* _sysbitInit: points the bit stream at arg0 at the arg3-byte ring buffer
   at arg2, reading from arg1, and resets it. The store order is
   load-bearing (LEVERS.md lever 3): arg1's +0x8 store has to come before
   its +0xC store, after the other fields in retail's order. */
void func_0012E890(void *arg0, int arg1, int arg2, int arg3) {
    char *p = (char *)arg0;
    *(int *)(p + 0x28) = arg3;
    *(int *)(p + 0x8) = arg1;
    *(long *)(p + 0x0) = 0;
    *(int *)(p + 0x10) = 0;
    *(long *)(p + 0x18) = 0;
    *(int *)(p + 0x20) = arg2;
    *(int *)(p + 0x24) = arg2 + arg3;
    *(int *)(p + 0xC) = arg1;
    r1_func_0012AAC8(arg0, 0);
}

INCLUDE_ASM("asm/core", func_0012E8C8);
