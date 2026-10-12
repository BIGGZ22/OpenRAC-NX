#include "include_asm.h"

extern void func_00115AC0(void *, int, int);

extern void func_00337E98(void *, int, int, float, float);

/* clamps a caller-supplied terminator value (-1 meaning unset) to 180, then runs two callees on two sub-objects of one handle, the second with a float pair */
void func_003387A0(int object, int value, int count)
{
    int limit = (count == -1) ? 180 : count;
    int inner = object + 7532;

    func_00115AC0((void *)(object + 7700), value, 254);
    func_00337E98((void *)inner, 1, limit, 255.0f, 123.0f);
}

void func_00338808(void)
{
}

INCLUDE_ASM("asm/game", func_00338810);

void func_00338A58(void)
{
}
