/* func_L00_002A75D8, 220 bytes, 16 placements.  ABANDON - compiler-profile wall, the same
   one as func_L00_003ACC88.
   One frame of the 124-mode camera block: when the resident mode word (+0x2294) is 124 it
   calls func_3231B8 on the resident pointer with three constants, then reads the +0x6BC
   short, and while it is set and +0x660 is clear it publishes the +0x6B0 float into the
   object's +0xF4 field and runs a three-call vector path over a stack buffer.
   No C is deposited: the retail materialises a 128-bit zero with an MMI pair
   (0x700014A9 = pcpyld $v0,$zero,$zero, then 0x7FA20000 = sq $v0,0x0($sp)), and
   rac2_fold_zero_ti_store folds that store to `sq $zero` because $v0 is dead afterwards
   (it is: the next statement only loads $a1). The register form cannot be produced from C
   under the pinned chain, exactly as recorded for func_L00_003ACC88. The two calls after
   the buffer are func_2EE970 (a1 = resident + 0xF0) and func_2EE5D8, then func_2EE108. */

#include "include_asm.h"

/* Stack buffer published as a quadword: TI128 zero plus two floats. */
typedef int TI128 __attribute__((mode(TI)));

extern char D_00189E20_2[];
extern void func_3231B8(void *a0, char *a1, char *a2, float f12, float f13, float f14);
extern void func_2EE970(void *a0, void *a1, void *a2);
extern void func_2EE5D8(void *a0, void *a1, void *a2);
extern void func_2EE108(void *a0, void *a1, void *a2);

void func_L00_002A75D8(void)
{
    char *base = D_00189E20_2;
    float buffer[4];

    if (*(int *)(base + 0x2294) == 124)
        func_3231B8(*(void **)(base + 0x2290), base + 0x7A8, base + 0x7AC,
                    0.3f, 0.11639999693632126f, -1.0f);
    if (*(short *)(base + 0x6BC) == 0)
        return;
    if (*(int *)(base + 0x660) != 0)
        return;
    *(float *)(*(char **)(base + 0x2290) + 0xF4) = *(float *)(base + 0x6B0);
    *(TI128 *)buffer = 0;
    buffer[2] = 2.0f;
    func_2EE970(buffer, *(char **)(base + 0x2290) + 0xF0, (char *)base + 0x10);
    func_2EE5D8(buffer, buffer, (char *)base + 0x10);
    func_2EE108(*(char **)(base + 0x2290) + 0x10, base + 0x80, buffer);
}
