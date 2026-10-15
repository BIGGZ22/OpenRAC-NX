#include "include_asm.h"

extern int D_L00_001A8F00;
extern int D_L00_001A9350 __attribute__((sda));
extern int D_L00_001A9090 __attribute__((sda));
extern int func_00282948(void *);
extern void func_0029B810(int);
int func_00290628(void)
{
    if (D_L00_001A8F00 == 0 && func_00282948(&D_L00_001A9350) != 0)
        func_0029B810(1);
    return D_L00_001A9090;
}

extern int D_001A9000 __attribute__((sda));
extern int D_001A9004 __attribute__((sda));
extern float D_001A9008 __attribute__((sda));
extern int D_001A900C __attribute__((sda));
extern int D_001A9010 __attribute__((sda));
extern void func_002900B8(void);
extern void func_0029B7C0(int (*)(void), void (*)(void), int);
extern float func_002AA340(int *, int, int);

void func_00290010(int mode, int first, int second, float amount)
{
    if (D_001A9000 == 0 || (D_001A9004 != 2 && mode == 2)) {
        D_001A9004 = mode;
        func_0029B7C0(func_00290628, func_002900B8, 1);
    }
    if (mode == D_001A9004) {
        D_001A900C = first;
        D_001A9010 = second;
        D_001A9008 = amount;
        func_002AA340(&D_001A9000, 10, 2);
    }
}
