#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

extern char *func_0029EFD0(int);
extern int func_002A0488(char *, int, int, int);
extern float func_002A7878(float, float);
extern void func_002A0F48(char *);

char *func_00308600(char *a0, char *a1, int a2, int a3, int t0)
{
    char *p = func_0029EFD0(0x7A);
    char *e;
    float lo = 0.1047197729f;
    float hi = 0.2094395458f;

    if (p == 0)
        return p;
    p[0x31] = 1;
    p[0x23] = (char)0x80;
    *(short *)(p + 0x32) = 0xFF;
    e = *(char **)(p + 0x68);
    p[0x30] = func_002A0488(p, 0x7F, 0x7F, 0x7F);
    *(TI128 *)(p + 0x10) = *(TI128 *)a0;
    *(TI128 *)e = *(TI128 *)a1;
    *(float *)(e + 0x10) = func_002A7878(lo, hi);
    *(float *)(e + 0x14) = func_002A7878(lo, hi);
    *(short *)(e + 0x18) = (short)a2;
    *(short *)(e + 0x1A) = (short)a2;
    if (a3 == 2) {
        a3 = 1;
        *(int *)(e + 0x20) = a3;
    }
    if (a3 == 0)
        *(float *)(p + 0x2C) = *(float *)(p + 0x2C) * func_002A7878(0.5f, 0.75f);
    *(float *)(e + 0x1C) = *(float *)(p + 0x2C);
    func_002A0F48(p);
    p[0xBC] = (char)(a3 | (t0 << 1));
    return p;
}
