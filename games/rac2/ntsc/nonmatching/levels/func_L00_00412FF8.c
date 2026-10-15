#include "include_asm.h"

extern int D_L00_001B1378 __attribute__((sda));
extern int D_L00_001B137C __attribute__((sda));
extern unsigned short D_L00_001B13DC __attribute__((sda));
extern char D_00199EB0[];
extern char D_0019C190[];
extern char D_L00_0025A1A0[];
extern char D_L00_0025A420[];
extern char *func_003100D8(int id);
extern void func_002833F0(void *dst, void *src);
extern void func_00282D30(void *dst, void *src);
extern void func_00282CC0(void *dst, void *src, int flags);
extern void func_L00_00413298(char *object);

int func_L00_00412FF8(char *owner)
{
    char *object = func_003100D8(0x1195);
    char *state;
    int counter;
    int next;
    int base;

    if (object == 0)
        return 1;

    *(unsigned char *)(object + 0x20) = 1;
    *(unsigned char *)(object + 0x30) = 0xFF;
    state = *(char **)(object + 0x68);
    *(int *)(state + 0x18) = -1;
    func_002833F0(object + 0xC0, D_00199EB0);
    *(unsigned short *)(state + 0x1E) = D_L00_001B13DC;

    counter = D_L00_001B1378;
    *(char **)(state + 0) = D_L00_0025A1A0 + 0xA0;
    *(char **)(state + 4) = D_L00_0025A420 + 0x140;
    if ((unsigned int)counter >= 4)
        D_L00_001B1378 = 0;

    counter = D_L00_001B1378;
    next = counter + 1;
    base = next >= 0 ? next : counter + 4;
    D_L00_001B1378 = next - ((base >> 2) << 2);

    *(int *)(state + 0x24) = D_L00_001B137C;
    *(char **)(state + 0x20) = owner;
    func_00282D30(&counter, D_0019C190 + 0xE0);
    func_00282CC0(object + 0x10, &counter, D_L00_001B137C);
    func_L00_00413298(object);
    return (int)object;
}
