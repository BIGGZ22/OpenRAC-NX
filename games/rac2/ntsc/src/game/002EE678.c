#include "include_asm.h"

/* func_002EE6A8: owner ABI/declaration review retained. */
extern int func_002ED688(unsigned char *owner);

#ifndef RAC2_T_V4_FB6552688
#define RAC2_T_V4_FB6552688
typedef int V4_Fb6552688 __attribute__((mode(TI)));
#endif

extern char *func_002ED688_1(char *);

extern void func_00282F20_10(V4_Fb6552688 *, char *, float);

extern void func_00282CC0_26(char *, V4_Fb6552688 *, char *);

extern void func_002EE988(char *);

void func_002EE678(unsigned char *owner, unsigned int value)
{
    unsigned int address = (unsigned int)func_002ED688(owner);
    if (address != 0)
        *(unsigned int *)(address + 0x100u) = value;
}

unsigned int func_002EE6A8(unsigned char *owner)
{
    unsigned int address = (unsigned int)func_002ED688(owner);
    if (address == 0)
        return 0;
    return *(unsigned int *)(address + 0x100u);
}

INCLUDE_ASM("asm/game", func_002EE6D0);

INCLUDE_ASM("asm/game", func_002EE6E0);

INCLUDE_ASM("asm/game", func_002EE7B8);

/* Calls a callee with the object, returns when it fails, otherwise fills a 16-byte workspace from the object at +192 with 8.0f, hands it to a second callee through +96, calls a third, then stores 300 at +304. */
void func_002EE8B0(char *a0)
{
    V4_Fb6552688 buf;
    char *p;

    p = func_002ED688_1(a0);
    if (p == 0)
        return;
    func_00282F20_10(&buf, a0 + 192, 8.0f);
    func_00282CC0_26(p + 96, &buf, a0 + 16);
    func_002EE988(a0);
    *(int *)(p + 304) = 300;
}

INCLUDE_ASM("asm/game", func_002EE920);

INCLUDE_ASM("asm/game", func_002EE988);
