#include "include_asm.h"

extern int func_L00_002D1DA0_1(int, void *, void *);

extern void func_002A7470(void *, int, int, int);

extern char D_00189E20_36[];

INCLUDE_ASM("asm/levels", func_L00_002CAD90);

INCLUDE_ASM("asm/levels", func_L00_002CAF20);

INCLUDE_ASM("asm/levels", func_L00_002CB038);

INCLUDE_ASM("asm/levels", func_L00_002CB218);

INCLUDE_ASM("asm/levels", func_L00_002CB2D8);

INCLUDE_ASM("asm/levels", func_L00_002CB3A8);

/* loads two object pointers from fixed data, bails out when either is null or when the first object's byte 67 differs from 1, fills two stack ints through a helper, then applies a four-argument operation to each object with those ints */
void func_L00_002D1AE8(void)
{
    char *p = D_00189E20_36;
    void *s0;
    void *s1;
    int buf[2];

    s0 = *(void **)(p + 4880);
    s1 = *(void **)(p + 4884);

    if (s0 == 0)
        return;
    if (s1 == 0)
        return;

    if (*(unsigned char *)((char *)s0 + 67) != 1)
        return;

    if (func_L00_002D1DA0_1(0, buf, buf + 1) == 0)
        return;

    func_002A7470(s0, buf[0], 0, 7);
    func_002A7470(s1, buf[1], 0, 7);
}

INCLUDE_ASM("asm/levels", func_L00_002D1B70);

INCLUDE_ASM("asm/levels", func_L00_002D1C40);

INCLUDE_ASM("asm/levels", func_L00_002D1DA0);
