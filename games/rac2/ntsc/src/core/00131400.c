#include "include_asm.h"

/* Qwen draft (branch qwen/autonomy, 3d350393), checked by tools/check.py */
extern void func_00123028(long long, long long);

int func_00131400(void *arg)
{
    long long val = *(long long *)(arg + 8);
    func_00123028(val, val);
    return 0;
}

INCLUDE_ASM("asm/core", func_00131424);

INCLUDE_ASM("asm/core", func_0013153C);
