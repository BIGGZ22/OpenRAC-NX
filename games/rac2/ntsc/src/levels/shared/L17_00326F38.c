#include "include_asm.h"

extern void func_2EF398(void *, void *, void *);

extern void func_2EEF40(void *, void *, float);

extern void func_2EEEB8(void *, void *, void *);

/* transforms a point given in three floats through the sub-object's matrix, rescales it and
   writes it back through the descriptor */
void func_L17_00326F38(char *a0, char *a1, float f12, float f13, float f14)
{
    float v[3];

    v[0] = f12;
    v[1] = f13;
    v[2] = f14;
    func_2EF398(v, v, a0 + 0xC0);
    func_2EEF40(v, v, *(float *)(a0 + 0x2C));
    func_2EEEB8(a1, a1, v);
}

INCLUDE_ASM("asm/levels", func_L17_0032B558);
