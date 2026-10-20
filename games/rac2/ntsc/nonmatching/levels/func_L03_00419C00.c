/* 0 of 2 places match - last line: 03_endako: instruction at +0x1c differs.
   Whole body equal (frame, save, the two frame addresses in $t1/$t2, both float
   constants, the call) except the two -1 sentinel stores: retail issues them at
   +0x38 and in the `jal` delay slot, this cc1 issues both directly after the
   state load.  Scheduling of two independent stores.
   The 9-argument prototype is the one measured in
   src/levels/02_maktar_nebula/L02_002ED7B0.c; the canonical unit declares the
   same callee in a 7-integer form that cannot express this call, which is why
   this function is kept in a file of its own. */

#include "include_asm.h"

typedef int s32;
typedef unsigned char u8;

extern u8 *D_L03_001B2A1C __attribute__((sda));
extern void func_L02_0032E998(char *, int, float *, char *, s32, char *, char *,
                              float, float);

/* hands the element's state block to the shared query with a zero/0.1 scale pair
   and two sentinel outputs */
void func_L03_00419C00(u8 *element, u8 *arg1, s32 arg2)
{
    u8 *q = *(u8 **)(element + 0x68);
    s32 out0;
    s32 out1 = -1;
    s32 out2 = -1;

    func_L02_0032E998((char *)arg1, arg2, (float *)&out0, (char *)(q + 0x80),
                      *(s32 *)(q + 0x480), (char *)&out1, (char *)&out2, 0.0f, 0.1f);
}
