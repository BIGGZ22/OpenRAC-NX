#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L07_00389EB8);

INCLUDE_ASM("asm/levels", func_L07_003B72E8);

INCLUDE_ASM("asm/levels", func_L07_003E4BD0);

INCLUDE_ASM("asm/levels", func_L07_003E5070);

INCLUDE_ASM("asm/levels", func_L07_003E5198);

INCLUDE_ASM("asm/levels", func_L07_003E56B8);

INCLUDE_ASM("asm/levels", func_L07_003E5910);

extern void func_00282978(char *);

/* Clear the two resource slots and step the state counter while it has room. */
void func_L07_003E5A70(char *arg)
{
    char *o = *(char **)(arg + 104);
    unsigned short v;

    func_00282978(o + 0x240);
    func_00282978(o + 0x242);
    v = *(unsigned short *)(arg + 0x96);
    if (v <= 0xFFFE)
        *(unsigned short *)(arg + 0x96) = v + 1;
}

INCLUDE_ASM("asm/levels", func_L07_003E5AC8);

INCLUDE_ASM("asm/levels", func_L07_003E5B70);

INCLUDE_ASM("asm/levels", func_L07_003E5C78);

INCLUDE_ASM("asm/levels", func_L07_003E5D80);

INCLUDE_ASM("asm/levels", func_L07_003E5FA8);

extern void func_002AD0E0(char *, int, char *, char *, char *, char *);

/* Run the shared pass on both halves of the object, then again on the
   secondary half with a stack buffer when the state byte reads nine. */
void func_L07_003E6148(char *arg)
{
    char *o = *(char **)(arg + 104);
    char tmp[16];

    func_002AD0E0(arg, *(int *)(o + 0x260), arg + 0x10, arg + 0xF0, arg + 0x10, arg + 0xF0);
    if (*(unsigned char *)(arg + 0x20) == 9)
        func_002AD0E0(arg, *(int *)(o + 0x260), o + 0x230, tmp, o + 0x230, tmp);
}

/* func_L07_003E61B8 is in the sibling file L07_00389EB8-g8.c: its body
   reaches a small-data global through $gp. */

INCLUDE_ASM("asm/levels", func_L07_003E61E0);

INCLUDE_ASM("asm/levels", func_L07_003E69B8);

INCLUDE_ASM("asm/levels", func_L07_003E6E00);

INCLUDE_ASM("asm/levels", func_L07_003E6F40);

INCLUDE_ASM("asm/levels", func_L07_003E6FE8);

INCLUDE_ASM("asm/levels", func_L07_003E7240);

extern void func_002A7750(int, int, int);

extern int func_002A7820(int, int);

extern void func_L07_002E2090(char *, int, int, char *, int, int, int);

/* Arm the object's mode ten and hand it to the shared launcher. */
void func_L07_003E7380(char *arg, int a1, int a2, int a3, int a4)
{
    int r;

    func_002A7750(arg, 10, -1);
    *(unsigned short *)(arg + 0x34) = (*(unsigned short *)(arg + 0x34) & 0xFFBE) | 0x1000;
    *(int *)(arg + 0x98) = *(int *)(*(char **)(arg + 0x24) + 0x10);
    r = func_002A7820(0x2D, 0x4B);
    func_L07_002E2090(arg, a1, a2, arg + 0xF0, r, a3, a4);
}

INCLUDE_ASM("asm/levels", func_L07_003EA900);

INCLUDE_ASM("asm/levels", func_L07_003EB2C0);

extern void func_00282948(char *);

extern void func_002B0990(char *, char *, int, int, float);

extern int D_L07_001C9780;

void func_L07_003EB700(char *arg)
{
    char *o = *(char **)(arg + 104);
    unsigned short v;

    func_00282948(o + 0x2D4);
    func_00282948(o + 0x2D8);
    v = *(unsigned short *)(arg + 0x96);
    if (v <= 0xFFFE)
        *(unsigned short *)(arg + 0x96) = v + 1;
    if (D_L07_001C9780 != 0)
        func_L07_003EB838(arg);
    func_002B0990(arg, o + 0x230, 5, 1, 2.5f);
}

INCLUDE_ASM("asm/levels", func_L07_003EB790);

INCLUDE_ASM("asm/levels", func_L07_003EB838);

INCLUDE_ASM("asm/levels", func_L07_003EBAB8);

/* Twin of func_L07_003E7380 with the mode eight instead of ten. */
void func_L07_003EBBE8(char *arg, int a1, int a2, int a3, int a4)
{
    int r;

    func_002A7750(arg, 8, -1);
    *(unsigned short *)(arg + 0x34) = (*(unsigned short *)(arg + 0x34) & 0xFFBE) | 0x1000;
    *(int *)(arg + 0x98) = *(int *)(*(char **)(arg + 0x24) + 0x10);
    r = func_002A7820(0x2D, 0x4B);
    func_L07_002E2090(arg, a1, a2, arg + 0xF0, r, a3, a4);
}

INCLUDE_ASM("asm/levels", func_L07_003EE3B0);

INCLUDE_ASM("asm/levels", func_L07_003EE640);

INCLUDE_ASM("asm/levels", func_L07_003EE708);

INCLUDE_ASM("asm/levels", func_L07_003EEBF8);

INCLUDE_ASM("asm/levels", func_L07_003EECE0);

/* Play the fixed mode/value pair for this object. */
void func_L07_003EEE08(char *arg)
{
    func_002A7750(arg, 5, -1);
}

INCLUDE_ASM("asm/levels", func_L07_003EEE28);

extern float func_002E5460(int);

extern void func_003109C8(char *, int, int);

/* Store the mode value and the inverse of the launch factor. */
/* func_L07_003EF1E0 is matched in L07_003EF1E0.c (built with -g). */

INCLUDE_ASM("asm/levels", func_L07_003F3E80);

INCLUDE_ASM("asm/levels", func_L07_003F43E0);
